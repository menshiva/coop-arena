#pragma once

#include "GameFramework/PlayerController.h"
#include "CoopArenaPlayerController.generated.h"

class UUserWidget;
class UInputMappingContext;

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

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> HudWidgetClass;

	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;
private:
	bool bUsingGamepad = false;
};
