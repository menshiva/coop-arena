#pragma once

#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "CoopArenaEnemyCharacter.generated.h"

class UAbilitySystemComponent;
class UCoopArenaAttributeSet;
class UWidgetComponent;
struct FOnAttributeChangeData;

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

	FORCEINLINE bool IsArrivedToStandingPlayer() const { return bArrivedToStandingPlayer; }
	FORCEINLINE void SetArrivedToStandingPlayer(const bool bValue) { bArrivedToStandingPlayer = bValue; }
protected:
	UPROPERTY(EditAnywhere, Category="Health")
	FInt32Interval MaxHealthRange = FInt32Interval(50, 150);

	UPROPERTY(EditAnywhere, Category="Health")
	int32 MaxHealth = 0;

	UPROPERTY(EditAnywhere, Category="Movement")
	FFloatInterval BaseSpeedRange = FFloatInterval(400.0f, 650.0f);

	UPROPERTY(EditAnywhere, Category="Movement")
	float BaseSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category="Movement")
	FFloatInterval SpeedPercentByDistanceRange = FFloatInterval(-0.15f, +0.15f);

	UPROPERTY(EditAnywhere, Category="Movement")
	FFloatInterval SpeedDistanceRange = FFloatInterval(300.0f, 1500.0f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UWidgetComponent> HealthWidget;
private:
	void OnHealthChanged(const FOnAttributeChangeData& Data) const;
	void OnDeath(AActor* Killer);

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystem;

	UPROPERTY()
	TObjectPtr<UCoopArenaAttributeSet> Attributes;

	bool bArrivedToStandingPlayer = false;
};
