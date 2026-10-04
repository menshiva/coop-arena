#pragma once

#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "CoopArenaProjectileManager.generated.h"

class UCoopArenaProjectileMovement;
class UInstancedStaticMeshComponent;

USTRUCT()
struct FCoopArenaBallSlot {
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY()
	TObjectPtr<UCoopArenaProjectileMovement> Movement;

	FGameplayEffectSpecHandle DamageSpec;
};

UCLASS(Abstract)
class ACoopArenaProjectileManager : public AActor {
	GENERATED_BODY()
public:
	ACoopArenaProjectileManager();
protected:
	virtual void BeginPlay() override;
public:
	void Launch(
		const FVector& Location, const FVector& Velocity, const FGameplayEffectSpecHandle& DamageSpec,
		AActor* ActorToIgnore
	);

	void OnBallImpact(int32 SlotIndex, const FHitResult& Hit);
	void OnBallStopped(int32 SlotIndex, bool bCreateIsmCopy);
protected:
	UPROPERTY(EditDefaultsOnly, Category="Ball")
	TSubclassOf<UStaticMeshComponent> BallComponentClass;

	UPROPERTY(EditDefaultsOnly, Category="Ball")
	TSubclassOf<UCoopArenaProjectileMovement> MovementComponentClass;

	UPROPERTY(EditInstanceOnly, Category="Ball", meta=(ClampMin=1))
	int32 BallsLimit = 500;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UInstancedStaticMeshComponent> BallsIsm;
private:
	UPROPERTY()
	TArray<FCoopArenaBallSlot> BallSlots;

	TSet<int32> BallSlotFreeIndices;

	int32 NextIsmIndex = 0;
};
