#pragma once

#include "CoopArenaGameplayAbility.h"
#include "CoopArenaGameplayAbility_Dash.generated.h"

UCLASS(Abstract)
class UCoopArenaGameplayAbility_Dash : public UCoopArenaGameplayAbility {
	GENERATED_BODY()
public:
	UCoopArenaGameplayAbility_Dash();
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData
	) override;

	UPROPERTY(EditDefaultsOnly, Category="Dash")
	TObjectPtr<UAnimMontage> DashMontage;

	UPROPERTY(EditDefaultsOnly, Category="Modifiers")
	float Strength = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category="Modifiers")
	float FlyingStrength = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category="Modifiers")
	float Duration = 0.2f;
};
