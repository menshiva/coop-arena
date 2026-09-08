#pragma once

#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "CoopArenaCharacter.generated.h"

class UInputAction;
class UCoopArenaGameplayAbility;
class UAbilitySystemComponent;
class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;
class UGameplayEffect;

USTRUCT()
struct FCoopArenaAbilityBinding {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> InputAction;

	UPROPERTY(EditAnywhere, meta = (Categories = "CoopArena.Ability"))
	FGameplayTag AbilityTag;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UCoopArenaGameplayAbility> AbilityClass;

	UPROPERTY(EditAnywhere)
	bool bGrantedAtStart = false;
};

UCLASS(Abstract)
class ACoopArenaCharacter : public ACharacter, public IAbilitySystemInterface {
	GENERATED_BODY()
public:
	ACoopArenaCharacter();

	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystem.Get(); }
protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TArray<FCoopArenaAbilityBinding> AbilityBindings;

	UPROPERTY(EditDefaultsOnly, Category="Abilities")
	TSubclassOf<UGameplayEffect> InitStatsEffect;
private:
	void OnAbilityInput(FGameplayTag AbilityTag);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem;
};
