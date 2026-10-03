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

	if (!ProjectileManagerCache.IsValid()) {
		if (TActorIterator<ACoopArenaProjectileManager> It(GetWorld()); It)
			ProjectileManagerCache = *It;
		if (!ProjectileManagerCache.IsValid())
			return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	CharacterPtr->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const auto TraceEnd = ViewLocation + ViewRotation.Vector() * 10000.0;

	FHitResult Hit;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(CoopArenaAttackAim), false, CharacterPtr);
	const auto AimPoint = GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation, TraceEnd, ECC_Visibility, Params)
		? Hit.ImpactPoint
		: TraceEnd;

	const auto SocketPos = CharacterPtr->GetMesh()->GetSocketLocation(SocketName);

	UGameplayStatics::FSuggestProjectileVelocityParameters TossParams(GetWorld(), SocketPos, AimPoint, Speed);
	TossParams.bFavorHighArc = false;
	TossParams.TraceOption = ESuggestProjVelocityTraceOption::DoNotTrace;
	TossParams.bAcceptClosestOnNoSolutions = true;

	FVector Velocity;
	UGameplayStatics::SuggestProjectileVelocity(TossParams, Velocity);

	ProjectileManagerCache->Launch(SocketPos, Velocity, MakeOutgoingGameplayEffectSpec(DamageEffect), CharacterPtr);
}

int32 UCoopArenaGameplayAbility_Attack::GetDamageAtDistance(const float Distance) const {
	return FMath::RoundToInt(FMath::GetMappedRangeValueClamped(
		FVector2f(DamageDistance.Min, DamageDistance.Max), FVector2f(DamageRange.Max, DamageRange.Min), Distance
	));
}
