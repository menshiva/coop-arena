#pragma once

#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "CoopArenaPlayerState.generated.h"

class UCoopArenaAttributeSet;

UCLASS()
class ACoopArenaPlayerState : public APlayerState, public IAbilitySystemInterface {
	GENERATED_BODY()
public:
	ACoopArenaPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystem; }

	FORCEINLINE UCoopArenaAttributeSet* GetAttributes() const { return Attributes; }
private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystem;

	UPROPERTY()
	TObjectPtr<UCoopArenaAttributeSet> Attributes;
};
