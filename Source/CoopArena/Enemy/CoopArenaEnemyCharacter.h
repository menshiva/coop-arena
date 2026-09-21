#pragma once

#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "CoopArenaEnemyCharacter.generated.h"

class UAbilitySystemComponent;
class UWidgetComponent;
struct FOnAttributeChangeData;
class UCoopArenaAttributeSet;

UENUM()
enum class ECoopArenaEnemyChaseRole : uint8 {
	Tail, // straight at the player
	Intercept, // attacks from the side. if caught behind, goes around him in a small arc
	CounterRun // attacks head-on. if caught behind, goes around in a big arc
};

USTRUCT()
struct FCoopArenaEnemyStats {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 MaxHealth = 10;

	// based on the player's speed
	UPROPERTY(EditAnywhere, meta=(ClampMin=0))
	float SpeedFactor = 1.0f;

	UPROPERTY(EditAnywhere)
	ECoopArenaEnemyChaseRole ChaseRole = ECoopArenaEnemyChaseRole::Tail;

	UPROPERTY(EditAnywhere, meta=(ClampMin=0))
	float OvertakeOrbitRadius = 0.0f;

	UPROPERTY(EditAnywhere, meta=(ClampMin=0))
	float OvertakeSpeedFactor = 1.0f;
};

UCLASS(Abstract)
class ACoopArenaEnemyCharacter : public ACharacter, public IAbilitySystemInterface {
	GENERATED_BODY()
public:
	ACoopArenaEnemyCharacter();

	virtual void PostActorCreated() override;

#if WITH_EDITOR
	virtual void PostEditImport() override;
#endif

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystem; }

	bool IsArrivedToStandingPlayer() const;
	bool IsRepackingAfterDeath() const;
	void SetArrivedToStandingPlayer(bool bValue);

	FORCEINLINE void SetStats(const FCoopArenaEnemyStats& InStats) { Stats = InStats; }
	FORCEINLINE ECoopArenaEnemyChaseRole GetChaseRole() const { return Stats.ChaseRole; }
	FORCEINLINE float GetOvertakeOrbitRadius() const { return Stats.OvertakeOrbitRadius; }

	FORCEINLINE bool IsOvertaking() const { return bOvertaking; }
	FORCEINLINE void SetOvertaking(const bool bValue) { bOvertaking = bValue; }
protected:
	UPROPERTY(EditInstanceOnly, Category="Stats")
	FCoopArenaEnemyStats Stats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UWidgetComponent> HealthWidget;
private:
	void OnHealthChanged(const FOnAttributeChangeData& Data) const;
	void OnDeath(AActor* Killer);

#if WITH_EDITOR
	void RollStatsFromSpawner();
#endif

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystem;

	UPROPERTY()
	TObjectPtr<UCoopArenaAttributeSet> Attributes;

	bool bOvertaking = false;

	bool bArrivedToStandingPlayer = false;
	uint32 EnemyDeathCountWhenArrived = 0;
};
