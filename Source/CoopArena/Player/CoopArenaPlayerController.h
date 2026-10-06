#pragma once

#include "GameFramework/PlayerController.h"
#include "CoopArenaPlayerController.generated.h"

class UCoopArenaHUDWidget;
class UInputMappingContext;
struct FOnAttributeChangeData;
class UCoopArenaAttributeSet;

DECLARE_MULTICAST_DELEGATE_OneParam(FCoopArenaOnInputDeviceChanged, bool /*bGamepad*/);

UCLASS(Abstract)
class ACoopArenaPlayerController : public APlayerController {
	GENERATED_BODY()
public:
	virtual bool InputKey(const FInputKeyEventArgs& Params) override;

	FORCEINLINE bool IsUsingGamepad() const { return bUsingGamepad; }

	FCoopArenaOnInputDeviceChanged OnInputDeviceChanged;
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UCoopArenaHUDWidget> HudWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="Input|Input Mappings")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;
private:
	void OnHealthChanged(const FOnAttributeChangeData& Data) const;

	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

	bool bUsingGamepad = false;

	UPROPERTY()
	TObjectPtr<UCoopArenaHUDWidget> Hud;

	TWeakObjectPtr<const UCoopArenaAttributeSet> Attributes;
};
