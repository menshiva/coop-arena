#include "CoopArenaPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"

void ACoopArenaPlayerController::BeginPlay() {
	Super::BeginPlay();

	if (IsLocalPlayerController()) {
		bUsingGamepad = FSlateApplication::IsInitialized() && FSlateApplication::Get().IsGamepadAttached();

		if (HudWidgetClass)
			if (const auto HudWidget = CreateWidget<UUserWidget>(this, HudWidgetClass))
				HudWidget->AddToViewport();
	}
}

void ACoopArenaPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
		if (const auto SubsystemPtr = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
			for (const auto CurrentContextPtr : DefaultMappingContexts)
				SubsystemPtr->AddMappingContext(CurrentContextPtr, 0);
}

bool ACoopArenaPlayerController::InputKey(const FInputKeyEventArgs& Params) {
	constexpr static float GamepadDeadZone = 0.25f;

	const auto bGamepad = Params.IsGamepad();
	const auto bUsed = Params.Event == IE_Axis
		? FMath::Abs(Params.AmountDepressed) > (bGamepad ? GamepadDeadZone : 0.0f)
		: Params.Event != IE_Released;
	if (bUsed && bUsingGamepad != bGamepad) {
		bUsingGamepad = bGamepad;
		OnInputDeviceChanged.Broadcast(bGamepad);
	}

	return Super::InputKey(Params);
}
