#pragma once

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "CoopArenaAttributeSet.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FCoopArenaOnDeath, AActor* /*Killer*/);

UCLASS()
class UCoopArenaAttributeSet : public UAttributeSet {
	GENERATED_BODY()
public:
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY()
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UCoopArenaAttributeSet, MaxHealth);

	UPROPERTY()
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UCoopArenaAttributeSet, Health);

	FCoopArenaOnDeath OnDeath;
private:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
