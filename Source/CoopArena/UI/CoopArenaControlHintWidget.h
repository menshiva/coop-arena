#pragma once

#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "CoopArenaControlHintWidget.generated.h"

class UAbilitySystemComponent;

UCLASS(Abstract, meta=(DisableNativeTick))
class UCoopArenaControlHintWidget : public UUserWidget {
	GENERATED_BODY()
protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintImplementableEvent, Category="Control Hint")
	void OnInputDeviceChanged(bool bGamepad);

	UFUNCTION(BlueprintImplementableEvent, Category="Control Hint")
	void OnCooldownStarted(FGameplayTag Tag, float Duration);
private:
	void OnCooldownTagChanged(FGameplayTag Tag, int32 Count);

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem;
};
