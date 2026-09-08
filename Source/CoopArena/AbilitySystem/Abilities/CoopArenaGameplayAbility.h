#pragma once

#include "CoopArenaGameplayTags.h"
#include "Abilities/GameplayAbility.h"
#include "CoopArenaGameplayAbility.generated.h"

UCLASS(Abstract)
class UCoopArenaGameplayAbility : public UGameplayAbility {
	GENERATED_BODY()
public:
	UCoopArenaGameplayAbility() {
		InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
		ActivationBlockedTags.AddTag(CoopArena_Status_Dead);
	}
};
