#pragma once

#include "GameFramework/Actor.h"
#include "CoopArenaEnemySpawner.generated.h"

class ACoopArenaEnemyCharacter;

UCLASS(Abstract)
class ACoopArenaEnemySpawner : public AActor {
	GENERATED_BODY()
public:
	ACoopArenaEnemySpawner();
protected:
	virtual void BeginPlay() override;
public:
	void SpawnEnemy();
protected:
	UPROPERTY(EditAnywhere, Category="Spawn")
	TSubclassOf<ACoopArenaEnemyCharacter> EnemyClass;

	UPROPERTY(EditAnywhere, Category="Spawn")
	FFloatInterval FallHeightRange = FFloatInterval(1000.0f, 2000.0f);

	UPROPERTY(EditDefaultsOnly, Category="Spawn")
	int32 EnemyNum = 1;
private:
	UFUNCTION()
	void OnEnemyDestroyed(AActor* Enemy);
};
