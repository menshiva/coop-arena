#pragma once

#include "CoopArenaPlayerController.generated.h"

class UInputMappingContext;

UCLASS(Abstract)
class ACoopArenaPlayerController : public APlayerController {
	GENERATED_BODY()
protected:
	virtual void SetupInputComponent() override;

	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;
};
