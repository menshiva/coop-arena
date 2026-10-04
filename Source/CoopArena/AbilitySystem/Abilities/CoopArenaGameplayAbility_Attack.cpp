#include "CoopArenaGameplayAbility_Attack.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/CoopArenaProjectileManager.h"

UCoopArenaGameplayAbility_Attack::UCoopArenaGameplayAbility_Attack() {
	FGameplayTagContainer Tags;
	Tags.AddTagFast(CoopArena_Ability_Attack_Player_Basic);
	SetAssetTags(Tags);
}

void UCoopArenaGameplayAbility_Attack::OnAttackEvent(FGameplayEventData) {
	const auto CharacterPtr = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!CharacterPtr || !CharacterPtr->GetController())
		return;
	const auto World = GetWorld();

	if (!ProjectileManagerCache.IsValid()) {
		if (TActorIterator<ACoopArenaProjectileManager> It(World); It)
			ProjectileManagerCache = *It;
		if (!ProjectileManagerCache.IsValid())
			return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	CharacterPtr->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const auto ViewDirection = ViewRotation.Vector();
	const auto TraceStart = ViewLocation + ViewDirection * FVector::DotProduct(CharacterPtr->GetActorLocation() - ViewLocation, ViewDirection); // from the pawn's depth
	const auto TraceEnd = ViewLocation + ViewDirection * 10000.0;

	FHitResult Hit;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(CoopArenaAttackAim), false, CharacterPtr);
	const auto AimPoint = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params)
		? Hit.ImpactPoint
		: TraceEnd;

	APawn* TargetPtr = nullptr;
	{
		// find target: of the pawns along the aim ray, the nearest to the crosshair
		TArray<FHitResult> PawnHits;
		World->SweepMultiByObjectType(
			PawnHits, TraceStart, AimPoint, FQuat::Identity, FCollisionObjectQueryParams(ECC_Pawn),
			FCollisionShape::MakeSphere(TargetSweepRadius), Params
		);
		double TargetCos = -1.0;
		for (const auto& PawnHit : PawnHits) {
			const auto PawnPtr = Cast<APawn>(PawnHit.GetActor());
			if (!PawnPtr)
				continue;

			const double Cos = FVector::DotProduct(ViewDirection, (PawnPtr->GetActorLocation() - ViewLocation).GetSafeNormal());
			if (Cos > TargetCos) {
				TargetCos = Cos;
				TargetPtr = PawnPtr;
			}
		}
	}

	const auto SocketPos = CharacterPtr->GetMesh()->GetSocketLocation(SocketName);
	FVector Velocity;
	{
		// compute velocity
		if (TargetPtr) {
			// to the target with lead

			const double FlightTime = FVector::Dist(SocketPos, TargetPtr->GetActorLocation()) / Speed;

			UGameplayStatics::SuggestProjectileVelocity_MovingTarget(World, Velocity, SocketPos, TargetPtr, FVector::ZeroVector, 0.0, FlightTime);
		}
		else {
			// an arc into the aim point

			UGameplayStatics::FSuggestProjectileVelocityParameters TossParams(World, SocketPos, AimPoint, Speed);
			TossParams.bFavorHighArc = false;
			TossParams.TraceOption = ESuggestProjVelocityTraceOption::DoNotTrace;
			TossParams.bAcceptClosestOnNoSolutions = true;

			UGameplayStatics::SuggestProjectileVelocity(TossParams, Velocity);
		}
	}

	ProjectileManagerCache->Launch(SocketPos, Velocity, MakeOutgoingGameplayEffectSpec(DamageEffect), CharacterPtr);
}

int32 UCoopArenaGameplayAbility_Attack::GetDamageAtDistance(const float Distance) const {
	return FMath::RoundToInt(FMath::GetMappedRangeValueClamped(
		FVector2f(DamageDistance.Min, DamageDistance.Max), FVector2f(DamageRange.Max, DamageRange.Min), Distance
	));
}
