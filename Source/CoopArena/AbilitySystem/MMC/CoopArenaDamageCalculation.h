#pragma once

#include "GameplayModMagnitudeCalculation.h"
#include "CoopArenaDamageCalculation.generated.h"

UCLASS()
class UCoopArenaDamageCalculation : public UGameplayModMagnitudeCalculation {
	GENERATED_BODY()
public:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
