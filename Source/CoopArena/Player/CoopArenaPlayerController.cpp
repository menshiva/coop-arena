#include "CoopArenaPlayerController.h"
#include "AbilitySystemGlobals.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystem/Attributes/CoopArenaAttributeSet.h"
#include "Benchmark/CoopArenaBenchComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "UI/CoopArenaHealthWidget.h"
#include "UI/CoopArenaHUDWidget.h"

bool ACoopArenaPlayerController::InputKey(const FInputKeyEventArgs& Params) {
	static constexpr float GamepadDeadZone = 0.25f;

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

void ACoopArenaPlayerController::BeginPlay() {
	Super::BeginPlay();

	if (IsLocalPlayerController()) {
		bUsingGamepad = FSlateApplication::IsInitialized() && FSlateApplication::Get().IsGamepadAttached();

		if (HudWidgetClass)
			Hud = CreateWidget<UCoopArenaHUDWidget>(this, HudWidgetClass);
		if (Hud)
			Hud->AddToViewport();

		const auto AbilitySystemPtr = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState);
		if (Hud && AbilitySystemPtr) {
			Attributes = AbilitySystemPtr->GetSet<UCoopArenaAttributeSet>();
			AbilitySystemPtr->GetGameplayAttributeValueChangeDelegate(UCoopArenaAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &ACoopArenaPlayerController::OnHealthChanged);
			AbilitySystemPtr->GetGameplayAttributeValueChangeDelegate(UCoopArenaAttributeSet::GetHealthAttribute()).AddUObject(this, &ACoopArenaPlayerController::OnHealthChanged);
			OnHealthChanged(FOnAttributeChangeData()); // PossessedBy may have applied the stats already
		}

		if (FParse::Param(FCommandLine::Get(), TEXT("bench")))
			NewObject<UCoopArenaBenchComponent>(this)->RegisterComponent();
	}
}

void ACoopArenaPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
		if (const auto SubsystemPtr = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
			for (const auto CurrentContextPtr : DefaultMappingContexts)
				SubsystemPtr->AddMappingContext(CurrentContextPtr, 0);
}

void ACoopArenaPlayerController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);

	if (InPawn)
		InPawn->OnDestroyed.AddUniqueDynamic(this, &ACoopArenaPlayerController::OnPawnDestroyed);
}

void ACoopArenaPlayerController::OnHealthChanged(const FOnAttributeChangeData&) const {
	if (Hud && Attributes.IsValid())
		if (const auto HealthWidgetPtr = Hud->GetHealthWidget())
			HealthWidgetPtr->SetHealth(FMath::RoundToInt(Attributes->GetHealth()), FMath::RoundToInt(Attributes->GetMaxHealth()));
}

void ACoopArenaPlayerController::OnPawnDestroyed(AActor*) {
	if (const auto WorldPtr = GetWorld(); WorldPtr && !WorldPtr->bIsTearingDown)
		if (const auto GameModePtr = WorldPtr->GetAuthGameMode())
			GameModePtr->RestartPlayer(this);
}
