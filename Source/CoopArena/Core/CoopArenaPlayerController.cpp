#include "CoopArenaPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystem/CoopArenaAttributeSet.h"
#include "GameFramework/PlayerState.h"
#include "UI/CoopArenaHealthWidget.h"
#include "UI/CoopArenaHudWidget.h"

void ACoopArenaPlayerController::BeginPlay() {
	Super::BeginPlay();

	if (IsLocalPlayerController()) {
		bUsingGamepad = FSlateApplication::IsInitialized() && FSlateApplication::Get().IsGamepadAttached();

		if (HudWidgetClass)
			Hud = CreateWidget<UCoopArenaHudWidget>(this, HudWidgetClass);
		if (Hud)
			Hud->AddToViewport();

		const auto AbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState);
		if (Hud && AbilitySystem) {
			Attributes = AbilitySystem->GetSet<UCoopArenaAttributeSet>();
			AbilitySystem->GetGameplayAttributeValueChangeDelegate(UCoopArenaAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &ACoopArenaPlayerController::OnHealthChanged);
			AbilitySystem->GetGameplayAttributeValueChangeDelegate(UCoopArenaAttributeSet::GetHealthAttribute()).AddUObject(this, &ACoopArenaPlayerController::OnHealthChanged);
			OnHealthChanged(FOnAttributeChangeData()); // PossessedBy may have applied the stats already
		}
	}
}

void ACoopArenaPlayerController::OnHealthChanged(const FOnAttributeChangeData&) const {
	if (Hud && Attributes.IsValid()) {
		if (const auto HealthWidget = Hud->GetHealthWidget())
			HealthWidget->SetHealth(FMath::RoundToInt(Attributes->GetHealth()), FMath::RoundToInt(Attributes->GetMaxHealth()));
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

	const bool bGamepad = Params.IsGamepad();
	const bool bUsed = Params.Event == IE_Axis
		? FMath::Abs(Params.AmountDepressed) > (bGamepad ? GamepadDeadZone : 0.0f)
		: Params.Event != IE_Released;
	if (bUsed && bUsingGamepad != bGamepad) {
		bUsingGamepad = bGamepad;
		OnInputDeviceChanged.Broadcast(bGamepad);
	}

	return Super::InputKey(Params);
}
