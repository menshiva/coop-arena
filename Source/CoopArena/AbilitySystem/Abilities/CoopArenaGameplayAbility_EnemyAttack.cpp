#include "CoopArenaGameplayAbility_EnemyAttack.h"
#include "AbilitySystemComponent.h"
#include "CoopArenaGameplayTags.h"
#include "EngineUtils.h"
#include "Algo/SelectRandomWeighted.h"
#include "Animation/CoopArenaAnimNotify_SendGameplayEvent.h"
#include "Character/CoopArenaPlayerCharacter.h"

UCoopArenaGameplayAbility_EnemyAttack::UCoopArenaGameplayAbility_EnemyAttack() {
	SetAssetTags(FGameplayTagContainer(CoopArena_Ability_Attack_Enemy_Basic));
}

int32 UCoopArenaGameplayAbility_EnemyAttack::GetDamage(const FGameplayEffectContextHandle&) const {
	const auto ChosenPtr = Algo::SelectRandomWeightedBy(DamageChances, [] (const TPair<int32, float>& P) { return P.Value; });
	return ChosenPtr ? ChosenPtr->Key : 1;
}

bool UCoopArenaGameplayAbility_EnemyAttack::WouldHit(const AActor& Target) const {
	const auto EnemyPtr = GetAvatarActorFromActorInfo();
	if (!EnemyPtr)
		return false;

	double HitTime = 0.0;
	if (AttackMontage) {
		// time of the hit notify in the montage
		for (const auto& Event : AttackMontage->Notifies) {
			if (const auto NotifyPtr = Cast<UCoopArenaAnimNotify_SendGameplayEvent>(Event.Notify)) {
				if (NotifyPtr->GetEventTag() == CoopArena_Event_Attack) {
					HitTime = Event.GetTriggerTime();
					break;
				}
			}
		}
	}

	// where the target will be at the notify if both keep their velocities
	const auto ToTargetAtHit = Target.GetActorLocation() - EnemyPtr->GetActorLocation() + (Target.GetVelocity() - EnemyPtr->GetVelocity()) * HitTime;

	return ToTargetAtHit.SizeSquared() <= FMath::Square(HitDistance);
}

void UCoopArenaGameplayAbility_EnemyAttack::OnAttackEvent(FGameplayEventData) {
	const auto EnemyPtr = GetAvatarActorFromActorInfo();
	const auto DamageSpec = MakeOutgoingGameplayEffectSpec(DamageEffect);
	if (!EnemyPtr || !DamageSpec.IsValid())
		return;

	const auto EnemyPos = EnemyPtr->GetActorLocation();
	const auto Forward = FVector2D(EnemyPtr->GetActorForwardVector());
	const float MinCos = FMath::Cos(FMath::DegreesToRadians(HitHalfAngle));

	for (TActorIterator<ACoopArenaPlayerCharacter> It(GetWorld()); It; ++It) {
		const auto ToPlayer = It->GetActorLocation() - EnemyPos;
		if (ToPlayer.SizeSquared() > FMath::Square(HitDistance))
			continue;
		if (FVector2D::DotProduct(Forward, FVector2D(ToPlayer).GetSafeNormal()) < MinCos)
			continue;

		if (const auto AbilitySystemPtr = It->GetAbilitySystemComponent())
			AbilitySystemPtr->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
	}
}
