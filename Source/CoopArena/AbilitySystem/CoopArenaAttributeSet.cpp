#include "CoopArenaAttributeSet.h"

void UCoopArenaAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const {
	ClampAttribute(Attribute, NewValue);
	Super::PreAttributeBaseChange(Attribute, NewValue);
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

void UCoopArenaAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const {
	if (Attribute == GetMaxHealthAttribute())
		NewValue = FMath::Max(NewValue, 0.0f);
	else if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
}
