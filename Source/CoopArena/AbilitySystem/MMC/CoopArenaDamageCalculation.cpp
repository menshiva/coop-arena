#include "CoopArenaDamageCalculation.h"
#include "AbilitySystem/Abilities/CoopArenaGameplayAbility_Attack.h"
#include "AbilitySystem/Abilities/CoopArenaGameplayAbility_EnemyAttack.h"

float UCoopArenaDamageCalculation::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const {
	const auto Context = Spec.GetContext();

	if (const auto Attack = Cast<UCoopArenaGameplayAbility_Attack>(Context.GetAbility()))
		if (const auto Hit = Context.GetHitResult())
			return -Attack->GetDamageAtDistance(FVector::Dist(Context.GetOrigin(), Hit->ImpactPoint));

	if (const auto EnemyAttack = Cast<UCoopArenaGameplayAbility_EnemyAttack>(Context.GetAbility()))
		return -EnemyAttack->RollDamage();

	return 0.0f;
}
