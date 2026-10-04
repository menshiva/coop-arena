#include "CoopArenaControlHintWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "CoopArenaGameplayTags.h"
#include "Core/CoopArenaPlayerController.h"
#include "GameFramework/PlayerState.h"

void UCoopArenaControlHintWidget::NativeOnInitialized() {
	Super::NativeOnInitialized();

	if (const auto PlayerPtr = GetOwningPlayer<ACoopArenaPlayerController>()) {
		PlayerPtr->OnInputDeviceChanged.AddUObject(this, &UCoopArenaControlHintWidget::OnInputDeviceChanged);
		OnInputDeviceChanged(PlayerPtr->IsUsingGamepad());
	}

	AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwningPlayerState());
	if (AbilitySystem.IsValid()) {
		AbilitySystem->RegisterGameplayTagEvent(CoopArena_Cooldown_Attack_Player_Basic).AddUObject(this, &UCoopArenaControlHintWidget::OnCooldownTagChanged);
		AbilitySystem->RegisterGameplayTagEvent(CoopArena_Cooldown_Dash).AddUObject(this, &UCoopArenaControlHintWidget::OnCooldownTagChanged);
	}
}

void UCoopArenaControlHintWidget::OnCooldownTagChanged(const FGameplayTag Tag, const int32 Count) {
	if (Count > 0) {
		const auto Cooldowns = AbilitySystem->GetActiveEffectsTimeRemainingAndDuration(
			FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(Tag))
		);
		OnCooldownStarted(Tag, Cooldowns.IsEmpty() ? 0.0f : Cooldowns[0].Value);
	}
}
