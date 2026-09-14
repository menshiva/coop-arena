#include "CoopArenaDamageCalculation.h"
#include "AbilitySystem/Abilities/CoopArenaGameplayAbility_Attack.h"

float UCoopArenaDamageCalculation::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const {
	const auto Context = Spec.GetContext();
	if (const auto Attack = Cast<UCoopArenaGameplayAbility_Attack>(Context.GetAbility()))
		if (const auto Hit = Context.GetHitResult())
			return -Attack->GetDamageAtDistance(FVector::Dist(Context.GetOrigin(), Hit->ImpactPoint));
	return 0.0f;
}
