#include "CoopArenaAttackRing.h"
#include "GameFramework/Controller.h"
#include "NavigationSystem.h"
#include "NavModifierComponent.h"

static TAutoConsoleVariable CVarDebugSlots(TEXT("CoopArena.DebugSlots"), false, TEXT("Draws the attack ring."));

UCoopArenaNavArea_RingZone::UCoopArenaNavArea_RingZone() {
	DefaultCost = 10.0f;
	DrawColor = FColor::Orange;
}

ACoopArenaAttackRing::ACoopArenaAttackRing() {
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Zone = CreateDefaultSubobject<UNavModifierComponent>(TEXT("Zone"));
	Zone->SetAreaClass(UCoopArenaNavArea_RingZone::StaticClass());
	Zone->SetNavigationRelevancy(false);
}

void ACoopArenaAttackRing::Tick(const float DeltaTime) {
	Super::Tick(DeltaTime);

	if (!Slots.IsEmpty() && CVarDebugSlots.GetValueOnGameThread()) {
		const auto Center = GetActorLocation();
		DrawDebugCircle(
			GetWorld(), Center, RingCreationDistance, 32, FColor::White, false,
			-1.0f, 0, 0.0f, FVector::ForwardVector, FVector::RightVector, false
		);
		DrawDebugBox(GetWorld(), Center, Zone->FailsafeExtent, FColor::Orange);
		for (const auto& Slot : Slots) {
			DrawDebugSphere(GetWorld(), Slot.Location, 20.0f, 8, Slot.Holder.IsValid() ? FColor::Green : FColor::Red);
			if (const auto PawnPtr = Slot.Holder.IsValid() ? Slot.Holder->GetPawn().Get() : nullptr)
				DrawDebugLine(GetWorld(), Slot.Location, PawnPtr->GetActorLocation(), FColor::Green);
		}
	}
}

bool ACoopArenaAttackRing::Acquire(const AController& Applicant, FVector& OutLocation) {
	const auto PawnPtr = Applicant.GetPawn().Get();
	if (!PawnPtr)
		return false;
	const auto TargetPtr = GetOwner();
	if (!TargetPtr)
		return false;
	const auto PawnPos = PawnPtr->GetActorLocation();
	const auto TargetPos = TargetPtr->GetActorLocation();

	if (!Slots.IsEmpty() && FVector::Dist2D(TargetPos, GetActorLocation()) > RingCreationDistance) {
		Slots.Reset();
		Zone->SetNavigationRelevancy(false);
	}

	if (Slots.IsEmpty()) {
		if (FVector::Dist2D(PawnPos, TargetPos) > RingCreationDistance)
			return false;

		const auto NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
		if (!NavSystem)
			return false;

		FNavLocation Center;
		if (!NavSystem->ProjectPointToNavigation(TargetPos, Center))
			return false;

		// init ring
		{
			const auto Direction = PawnPos - Center.Location;
			const double Start = FMath::Atan2(Direction.Y, Direction.X);

			Slots.Reset(RingEnemyMaxNum);
			for (int32 i = 0; i < RingEnemyMaxNum; ++i) {
				const double Angle = Start + UE_DOUBLE_TWO_PI * i / RingEnemyMaxNum;
				const auto Location = Center.Location + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * RingRadius;

				// the slot itself must be on the navmesh, on the owner's floor
				FNavLocation Projected;
				if (NavSystem->ProjectPointToNavigation(Location, Projected, FVector(10.0f, 10.0f, RingSlotHeightTolerance)))
					Slots.Emplace(Projected.Location, nullptr);
			}
			if (Slots.IsEmpty())
				return false;

			SetActorLocation(Center.Location);
			Zone->FailsafeExtent = FVector(NavZoneFailsafeExtent);
			Zone->UpdateNavigationBounds();
			Zone->SetNavigationRelevancy(true);
		}
	}

	auto SlotPtr = FindSlot(Applicant);
	if (!SlotPtr) {
		// nearest free slot
		double NearestDistance = TNumericLimits<double>::Max();
		for (auto& Slot : Slots) {
			if (Slot.Holder.IsValid())
				continue;
			const double Distance = FVector::Dist2D(Slot.Location, PawnPos);
			if (Distance < NearestDistance) {
				SlotPtr = &Slot;
				NearestDistance = Distance;
			}
		}
		if (!SlotPtr)
			return false;
		SlotPtr->Holder = &Applicant;
	}

	OutLocation = SlotPtr->Location;
	return true;
}

void ACoopArenaAttackRing::Release(const AController& Applicant) {
	if (const auto SlotPtr = FindSlot(Applicant))
		SlotPtr->Holder.Reset();
}

ACoopArenaAttackRing::FSlot* ACoopArenaAttackRing::FindSlot(const AController& Holder) {
	return Slots.FindByPredicate([&Holder] (const FSlot& Slot) { return Slot.Holder == &Holder; });
}
