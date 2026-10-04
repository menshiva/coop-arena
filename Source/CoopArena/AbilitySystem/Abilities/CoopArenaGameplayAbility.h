#pragma once

#include "Abilities/GameplayAbility.h"
#include "CoopArenaGameplayAbility.generated.h"

UCLASS(Abstract)
class UCoopArenaGameplayAbility : public UGameplayAbility {
	GENERATED_BODY()
public:
	UCoopArenaGameplayAbility() {
		InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
		bRetriggerInstancedAbility = true;
	}
};
