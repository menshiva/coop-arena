#include "CoopArenaAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Kismet/KismetSystemLibrary.h"

void UCoopArenaAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const {
	ClampAttribute(Attribute, NewValue);
	Super::PreAttributeBaseChange(Attribute, NewValue);
}

void UCoopArenaAttributeSet::PostAttributeBaseChange(const FGameplayAttribute& Attribute, const float OldValue, const float NewValue) const {
	Super::PostAttributeBaseChange(Attribute, OldValue, NewValue);

	if (Attribute == GetHealthAttribute() && NewValue < OldValue) {
		UKismetSystemLibrary::PrintString(
			this, FString::Printf(TEXT("took %d damage, %d health left"), FMath::RoundToInt(OldValue - NewValue), FMath::RoundToInt(NewValue)),
			true, true, FLinearColor::Red, 5.0f
		);
	}
}

void UCoopArenaAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) {
	ClampAttribute(Attribute, NewValue);
	Super::PreAttributeChange(Attribute, NewValue);
}

void UCoopArenaAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, const float OldValue, const float NewValue) {
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute())
		if (GetHealth() > NewValue)
			SetHealth(NewValue);
}

void UCoopArenaAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) {
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute() && GetHealth() <= 0.01f)
		OnDeath.Broadcast(Data.EffectSpec.GetContext().GetOriginalInstigator());
}

void UCoopArenaAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const {
	if (Attribute == GetMaxHealthAttribute())
		NewValue = FMath::Max(NewValue, 0.0f);
	else if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
}
