#include "CoopArenaPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"

void ACoopArenaPlayerController::BeginPlay() {
	Super::BeginPlay();

	if (IsLocalPlayerController())
		if (HudWidgetClass)
			if (const auto HudWidget = CreateWidget<UUserWidget>(this, HudWidgetClass))
				HudWidget->AddToViewport();
}

void ACoopArenaPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
		if (const auto SubsystemPtr = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
			for (const auto CurrentContextPtr : DefaultMappingContexts)
				SubsystemPtr->AddMappingContext(CurrentContextPtr, 0);
}
