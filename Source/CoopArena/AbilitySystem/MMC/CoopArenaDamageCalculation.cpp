#include "CoopArenaDamageCalculation.h"
#include "AbilitySystem/Abilities/CoopArenaGameplayAbility_AttackBase.h"

float UCoopArenaDamageCalculation::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const {
	const auto Context = Spec.GetContext();
	if (const auto AttackPtr = Cast<UCoopArenaGameplayAbility_AttackBase>(Context.GetAbility()))
		return -AttackPtr->GetDamage(Context);
	return 0.0f;
}
