#pragma once

#include "CoopArenaEnemyCharacter.h"
#include "GameFramework/Actor.h"
#include "CoopArenaEnemySpawner.generated.h"

UCLASS(Abstract)
class ACoopArenaEnemySpawner : public AActor {
	GENERATED_BODY()
public:
	ACoopArenaEnemySpawner();
protected:
	virtual void BeginPlay() override;
public:
	void SpawnEnemy();
	FCoopArenaEnemyStats GetEnemyNewRolledStats() const;
protected:
	UPROPERTY(EditDefaultsOnly, Category="Spawn")
	TSubclassOf<ACoopArenaEnemyCharacter> EnemyClass;

	UPROPERTY(EditInstanceOnly, Category="Spawn")
	FFloatInterval FallHeightRange = FFloatInterval(1000.0f, 4000.0f);

	UPROPERTY(EditInstanceOnly, Category="Spawn", meta=(ClampMin=0))
	int32 EnemyNum = 1;

	UPROPERTY(EditInstanceOnly, Category="Stats")
	FInt32Interval MaxHealthRange = FInt32Interval(8, 20);

	UPROPERTY(EditInstanceOnly, Category="Stats")
	FFloatInterval SpeedFactorRange = FFloatInterval(0.8f, 1.05f);

	UPROPERTY(EditInstanceOnly, Category="Stats|Intercept", meta=(ClampMin=0, ClampMax=1))
	float InterceptShare = 0.35f;

	UPROPERTY(EditInstanceOnly, Category="Stats|Intercept")
	FFloatInterval InterceptOrbitRadiusRange = FFloatInterval(150.0f, 250.0f);

	UPROPERTY(EditInstanceOnly, Category="Stats|Intercept")
	FFloatInterval InterceptOvertakeSpeedFactorRange = FFloatInterval(1.1f, 1.3f);

	UPROPERTY(EditInstanceOnly, Category="Stats|CounterRun", meta=(ClampMin=0, ClampMax=1))
	float CounterRunShare = 0.15f;

	UPROPERTY(EditInstanceOnly, Category="Stats|CounterRun")
	FFloatInterval CounterRunOrbitRadiusRange = FFloatInterval(400.0f, 600.0f);

	UPROPERTY(EditInstanceOnly, Category="Stats|CounterRun")
	FFloatInterval CounterRunOvertakeSpeedFactorRange = FFloatInterval(1.4f, 1.6f);
private:
	UFUNCTION()
	void OnEnemyDestroyed(AActor* Enemy);
};
