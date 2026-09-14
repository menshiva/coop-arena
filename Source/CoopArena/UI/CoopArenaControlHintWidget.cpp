#include "CoopArenaControlHintWidget.h"
#include "AbilitySystemComponent.h"
#include "CoopArenaGameplayTags.h"
#include "Core/CoopArenaPlayerController.h"
#include "GameFramework/PlayerState.h"

void UCoopArenaControlHintWidget::NativeOnInitialized() {
	Super::NativeOnInitialized();

	if (const auto Player = GetOwningPlayer<ACoopArenaPlayerController>()) {
		Player->OnInputDeviceChanged.AddUObject(this, &UCoopArenaControlHintWidget::OnInputDeviceChanged);
		OnInputDeviceChanged(Player->IsUsingGamepad());
	}

	AbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerState());
	if (AbilitySystem.IsValid()) {
		AbilitySystem->RegisterGameplayTagEvent(CoopArena_Cooldown_Attack).AddUObject(this, &UCoopArenaControlHintWidget::OnCooldownTagChanged);
		AbilitySystem->RegisterGameplayTagEvent(CoopArena_Cooldown_Dash).AddUObject(this, &UCoopArenaControlHintWidget::OnCooldownTagChanged);
	}
}

void UCoopArenaControlHintWidget::OnCooldownTagChanged(const FGameplayTag Tag, const int32 Count) {
	if (Count == 0)
		return;

	const auto Cooldowns = AbilitySystem->GetActiveEffectsTimeRemainingAndDuration(
		FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(Tag))
	);
	OnCooldownStarted(Tag, Cooldowns.IsEmpty() ? 0.0f : Cooldowns[0].Value);
}
