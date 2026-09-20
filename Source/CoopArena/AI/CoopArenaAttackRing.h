#pragma once

#include "GameFramework/Actor.h"
#include "NavAreas/NavArea.h"
#include "CoopArenaAttackRing.generated.h"

class AController;
class UNavModifierComponent;

UCLASS()
class UCoopArenaNavArea_RingZone : public UNavArea {
	GENERATED_BODY()
public:
	UCoopArenaNavArea_RingZone();
};

UCLASS(Blueprintable)
class ACoopArenaAttackRing : public AActor {
	GENERATED_BODY()
public:
	ACoopArenaAttackRing();

	virtual void Tick(float DeltaTime) override;

	bool Acquire(const AController& Applicant, FVector& OutLocation);
	void Release(const AController& Applicant);
protected:
	UPROPERTY(EditDefaultsOnly, Category="Ring")
	float RingCreationDistance = 250.0f;

	UPROPERTY(EditDefaultsOnly, Category="Ring")
	float RingRadius = 150.0f;

	UPROPERTY(EditDefaultsOnly, Category="Ring", meta=(ClampMin=1))
	int32 RingEnemyMaxNum = 6;

	UPROPERTY(EditDefaultsOnly, Category="Ring")
	float RingSlotHeightTolerance = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category="Ring")
	float NavZoneFailsafeExtent = 150.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UNavModifierComponent> Zone;
private:
	struct FSlot {
		FVector Location;
		TWeakObjectPtr<const AController> Holder;
	};
	TArray<FSlot> Slots;

	FSlot* FindSlot(const AController& Holder);
};
