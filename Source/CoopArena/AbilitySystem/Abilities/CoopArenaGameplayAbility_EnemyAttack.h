#pragma once

#include "CoopArenaGameplayAbility.h"
#include "CoopArenaGameplayAbility_EnemyAttack.generated.h"

UCLASS(Abstract)
class UCoopArenaGameplayAbility_EnemyAttack : public UCoopArenaGameplayAbility {
	GENERATED_BODY()
public:
	UCoopArenaGameplayAbility_EnemyAttack();

	bool WouldHit(const AActor& Target) const;

	int32 RollDamage() const;
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData
	) override;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, Category="Attack", meta=(ClampMin=0))
	float HitDistance = 150.0f;

	UPROPERTY(EditDefaultsOnly, Category="Attack", meta=(ClampMin=0, ClampMax=180))
	float HitHalfAngle = 45.0f;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	TMap<int32, float> DamageChances = {{1, 55.0f}, {2, 35.0f}, {3, 10.0f}};
private:
	UFUNCTION()
	void OnAttackEvent(FGameplayEventData Payload) const;

	float GetHitTime() const;
};
