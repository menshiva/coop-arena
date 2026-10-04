#pragma once

#include "GameFramework/ProjectileMovementComponent.h"
#include "CoopArenaProjectileMovement.generated.h"

class ACoopArenaProjectileManager;

UCLASS(Abstract, Blueprintable)
class UCoopArenaProjectileMovement : public UProjectileMovementComponent {
	GENERATED_BODY()
public:
	UCoopArenaProjectileMovement();

	void Init(ACoopArenaProjectileManager* InManager, int32 InSlotIndex);
protected:
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta) override;
public:
	virtual bool CheckStillInWorld() override;
	virtual void StopSimulating(const FHitResult& HitResult) override;
private:
	TWeakObjectPtr<ACoopArenaProjectileManager> Manager;

	int32 SlotIndex = INDEX_NONE;
};
