#pragma once

#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "CoopArenaEnemyCharacter.generated.h"

class UAbilitySystemComponent;
class UCoopArenaAttributeSet;

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

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystem; }
protected:
	UPROPERTY(EditAnywhere, Category="Health")
	FFloatInterval MaxHealthRange = FFloatInterval(50.0f, 150.0f);

	UPROPERTY(EditAnywhere, Category="Health")
	float MaxHealth = 0.0f;
private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystem;

	UPROPERTY()
	TObjectPtr<UCoopArenaAttributeSet> Attributes;
};
