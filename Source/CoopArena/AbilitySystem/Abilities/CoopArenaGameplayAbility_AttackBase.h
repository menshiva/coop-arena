#pragma once

#include "CoopArenaGameplayAbility.h"
#include "CoopArenaGameplayAbility_AttackBase.generated.h"

UCLASS(Abstract)
class UCoopArenaGameplayAbility_AttackBase : public UCoopArenaGameplayAbility {
	GENERATED_BODY()
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData
	) override;

	UFUNCTION()
	virtual void OnAttackEvent(FGameplayEventData Payload) {}

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	TSubclassOf<UGameplayEffect> DamageEffect;
};
