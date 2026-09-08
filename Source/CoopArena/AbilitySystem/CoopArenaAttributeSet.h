#pragma once

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "CoopArenaAttributeSet.generated.h"

UCLASS()
class UCoopArenaAttributeSet : public UAttributeSet {
	GENERATED_BODY()
public:
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	UPROPERTY()
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UCoopArenaAttributeSet, MaxHealth)

	UPROPERTY()
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UCoopArenaAttributeSet, Health)
private:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
