#include "CoopArenaPlayerController.h"
#include "EnhancedInputSubsystems.h"

void ACoopArenaPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController()) {
		// Add Input Mapping Contexts
		if (const auto SubsystemPtr = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
			for (const auto CurrentContextPtr : DefaultMappingContexts)
				SubsystemPtr->AddMappingContext(CurrentContextPtr, 0);
	}
}
