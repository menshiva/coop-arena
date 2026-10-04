#pragma once

#include "CoopArenaGameplayAbility_AttackBase.h"
#include "CoopArenaGameplayAbility_Attack.generated.h"

class ACoopArenaProjectileManager;

UCLASS(Abstract)
class UCoopArenaGameplayAbility_Attack : public UCoopArenaGameplayAbility_AttackBase {
	GENERATED_BODY()
public:
	UCoopArenaGameplayAbility_Attack();

	virtual int32 GetDamage(const FGameplayEffectContextHandle& Context) const override;
protected:
	virtual void OnAttackEvent(FGameplayEventData Payload) override;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	FName SocketName;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	float Speed = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	float TargetSweepRadius = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	FInt32Interval DamageRange = FInt32Interval(10, 20);

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	FFloatInterval DamageDistance = FFloatInterval(500.0f, 2000.0f);
private:
	TWeakObjectPtr<ACoopArenaProjectileManager> ProjectileManagerCache;
};
