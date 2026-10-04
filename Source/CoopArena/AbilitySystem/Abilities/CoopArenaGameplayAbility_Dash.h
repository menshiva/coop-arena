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

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled
	) override;

	UPROPERTY(EditDefaultsOnly, Category="Dash")
	TObjectPtr<UAnimMontage> DashMontage;

	UPROPERTY(EditDefaultsOnly, Category="Dash")
	float Strength = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category="Dash")
	float FlyingStrength = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category="Dash")
	float Duration = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category="Dash")
	float PushStrength = 600.0f;

	UPROPERTY(EditDefaultsOnly, Category="Dash")
	float PushUpStrength = 200.0f;
private:
	UFUNCTION()
	void PushTouched(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

	FVector DashDirection = FVector::ZeroVector;

	// this is mandatory: PushTouched can be called multiple times for the same actor
	TSet<TObjectKey<AActor>> PushedActorsSet;
};
