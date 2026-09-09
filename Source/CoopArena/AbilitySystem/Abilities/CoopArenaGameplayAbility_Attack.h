#pragma once

#include "CoopArenaGameplayAbility.h"
#include "CoopArenaGameplayAbility_Attack.generated.h"

class ACoopArenaProjectileManager;

UCLASS(Abstract)
class UCoopArenaGameplayAbility_Attack : public UCoopArenaGameplayAbility {
	GENERATED_BODY()
public:
	UCoopArenaGameplayAbility_Attack();
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData
	) override;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	FName MuzzleSocket;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	float AimRange = 10000.0f;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	float TossSpeed = 2000.0f;
private:
	UFUNCTION()
	void OnAttackEvent(FGameplayEventData Payload);

	UPROPERTY()
	TWeakObjectPtr<ACoopArenaProjectileManager> ProjectileManagerCache;
};
