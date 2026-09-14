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
protected:
	UPROPERTY(EditAnywhere, Category="Health")
	FInt32Interval MaxHealthRange = FInt32Interval(50, 150);

	UPROPERTY(EditAnywhere, Category="Health")
	int32 MaxHealth = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UWidgetComponent> HealthWidget;
private:
	void OnHealthChanged(const FOnAttributeChangeData& Data) const;
	void OnDeath(AActor* Killer);

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystem;

	UPROPERTY()
	TObjectPtr<UCoopArenaAttributeSet> Attributes;
};
