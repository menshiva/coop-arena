#include "BTService_CoopArenaApproach.h"
#include "AIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/CoopArenaEnemyCharacter.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Core/CoopArenaGameState.h"

static uint32 GetEnemyDeathCount(const UWorld* World) {
	if (const auto GameState = World->GetGameState<ACoopArenaGameState>())
		return GameState->GetEnemyDeathCount();
	return 0;
}

UBTService_CoopArenaApproach::UBTService_CoopArenaApproach() {
	NodeName = "Approach";
	Interval = 0.25f;

	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CoopArenaApproach, TargetActorKey), AActor::StaticClass());
	ApproachLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CoopArenaApproach, ApproachLocationKey));
	ArrivedKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CoopArenaApproach, ArrivedKey));
}

void UBTService_CoopArenaApproach::InitializeFromAsset(UBehaviorTree& Asset) {
	Super::InitializeFromAsset(Asset);

	if (const auto BlackboardPtr = GetBlackboardAsset()) {
		TargetActorKey.ResolveSelectedKey(*BlackboardPtr);
		ApproachLocationKey.ResolveSelectedKey(*BlackboardPtr);
		ArrivedKey.ResolveSelectedKey(*BlackboardPtr);
	}
}

void UBTService_CoopArenaApproach::InitializeMemory(UBehaviorTreeComponent&, uint8* NodeMemory, const EBTMemoryInit::Type InitType) const {
	InitializeNodeMemory<FMemory>(NodeMemory, InitType);
}

void UBTService_CoopArenaApproach::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, const float DeltaSeconds) {
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const auto BlackboardPtr = OwnerComp.GetBlackboardComponent();
	if (!BlackboardPtr)
		return;
	const auto TargetActorKeyID = TargetActorKey.GetSelectedKeyID();
	const auto LocationKeyID = ApproachLocationKey.GetSelectedKeyID();
	const auto ArrivedKeyID = ArrivedKey.GetSelectedKeyID();
	auto& Memory = *CastInstanceNodeMemory<FMemory>(NodeMemory);

	const auto ControllerPtr = OwnerComp.GetAIOwner();
	const auto EnemyPtr = ControllerPtr ? Cast<ACoopArenaEnemyCharacter>(ControllerPtr->GetPawn().Get()) : nullptr;
	const auto PlayerPtr = Cast<AActor>(BlackboardPtr->GetValue<UBlackboardKeyType_Object>(TargetActorKeyID));
	if (!EnemyPtr || !PlayerPtr) {
		BlackboardPtr->ClearValue(LocationKeyID);
		BlackboardPtr->ClearValue(ArrivedKeyID);
		if (EnemyPtr)
			EnemyPtr->SetArrivedToStandingPlayer(false);
		Memory.StalledFor = 0.0f;
		return;
	}
	const auto NavSystem = UNavigationSystemV1::GetCurrent(OwnerComp.GetWorld());

	if (!HandleOffNavmesh(NavSystem, *EnemyPtr, Memory))
		return;

	// approach
	BlackboardPtr->SetValue<UBlackboardKeyType_Vector>(LocationKeyID, PlayerPtr->GetActorLocation());

	// arrived
	BlackboardPtr->SetValue<UBlackboardKeyType_Bool>(ArrivedKeyID, IsArrived(*EnemyPtr, *PlayerPtr, Memory, DeltaSeconds));
}

bool UBTService_CoopArenaApproach::HandleOffNavmesh(const UNavigationSystemV1* NavSystem, ACoopArenaEnemyCharacter& Enemy, FMemory& Memory) {
	const auto EnemyPos = Enemy.GetActorLocation();
	FNavLocation OnNavmesh;
	if (NavSystem && Enemy.GetCharacterMovement()->IsMovingOnGround() && !NavSystem->ProjectPointToNavigation(EnemyPos, OnNavmesh)) {
		if (++Memory.OffNavmeshTicks < 4)
			return false;
		Memory.OffNavmeshTicks = 0;
		if (NavSystem->ProjectPointToNavigation(EnemyPos, OnNavmesh, FVector(500.0))) {
			// nudge back to the closest point
			Enemy.SetActorLocation(
				OnNavmesh.Location + FVector(0.0, 0.0, Enemy.GetSimpleCollisionHalfHeight()), false,
				nullptr, ETeleportType::TeleportPhysics
			);
		}
		else {
			Enemy.Destroy();
		}
		return false;
	}
	Memory.OffNavmeshTicks = 0;
	return true;
}

bool UBTService_CoopArenaApproach::IsArrived(ACoopArenaEnemyCharacter& Enemy, const AActor& Player, FMemory& Memory, const float DeltaSeconds) const {
	if (!Player.GetVelocity().IsNearlyZero() || !Enemy.GetCharacterMovement()->IsMovingOnGround()) {
		Enemy.SetArrivedToStandingPlayer(false);
		Memory.StalledFor = 0.0f;
		return false;
	}
	if (Enemy.IsArrivedToStandingPlayer())
		return true;

	const auto Arrive = [&] {
		Enemy.SetArrivedToStandingPlayer(true);
		Memory.StalledFor = 0.0f;
		return true;
	};

	if (Enemy.GetVelocity().SizeSquared2D() < FMath::Square(SpeedConsideredAsStall))
		Memory.StalledFor += DeltaSeconds;
	else
		Memory.StalledFor = 0.0f;

	if (Memory.StalledFor >= TimeConsideredAsStall)
		return Arrive();

	// an enemy death dropped the latch - give the crowd a moment to move into the gap before the neighbors anchor it again
	bool bNeighboursAnchor = true;
	if (Enemy.IsRepackingAfterDeath()) {
		const uint32 DeathCount = GetEnemyDeathCount(GetWorld());
		if (Memory.RepackingDeathCount != DeathCount) {
			// every further death restarts the window
			Memory.RepackingDeathCount = DeathCount;
			Memory.RepackingFor = 0.0f;
		}
		Memory.RepackingFor += DeltaSeconds;
		bNeighboursAnchor = Memory.RepackingFor >= TimeToRepackAfterDeath;
	}

	TArray<FOverlapResult> Overlaps;
	Enemy.GetWorld()->OverlapMultiByObjectType(
		Overlaps, Enemy.GetActorLocation(), FQuat::Identity, FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Enemy.GetSimpleCollisionRadius() + TouchPackingGapDistance),
		FCollisionQueryParams(NAME_None, false, &Enemy)
	);
	for (const auto& Overlap : Overlaps) {
		if (Overlap.GetActor() == &Player)
			return Arrive();

		if (bNeighboursAnchor)
			if (const auto OtherPtr = Cast<ACoopArenaEnemyCharacter>(Overlap.GetActor()))
				if (OtherPtr->IsArrivedToStandingPlayer())
					return Arrive();
	}

	return false;
}
