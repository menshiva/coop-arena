#pragma once

#include "CoopArenaGameplayAbility_AttackBase.h"
#include "CoopArenaGameplayAbility_EnemyAttack.generated.h"

UCLASS(Abstract)
class UCoopArenaGameplayAbility_EnemyAttack : public UCoopArenaGameplayAbility_AttackBase {
	GENERATED_BODY()
public:
	UCoopArenaGameplayAbility_EnemyAttack();

	virtual int32 GetDamage(const FGameplayEffectContextHandle& Context) const override;

	bool WouldHit(const AActor& Target) const;
protected:
	virtual void OnAttackEvent(FGameplayEventData Payload) override;

	UPROPERTY(EditDefaultsOnly, Category="Attack", meta=(ClampMin=0))
	float HitDistance = 150.0f;

	UPROPERTY(EditDefaultsOnly, Category="Attack", meta=(ClampMin=0, ClampMax=180))
	float HitHalfAngle = 45.0f;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	TMap<int32, float> DamageChances = {{1, 55.0f}, {2, 35.0f}, {3, 10.0f}};
};
