#pragma once

#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "CoopArenaCharacter.generated.h"

class UInputAction;
class UCoopArenaGameplayAbility;
class UAbilitySystemComponent;
class ACoopArenaAttackRing;
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

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystem.Get(); }
	FORCEINLINE ACoopArenaAttackRing* GetAttackRing() const { return AttackRing; }
protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UCameraComponent> FollowCamera;

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

	UPROPERTY(EditDefaultsOnly, Category="AI")
	TSubclassOf<ACoopArenaAttackRing> AttackRingClass;
private:
	void OnAbilityInput(FGameplayTag AbilityTag);

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem;

	UPROPERTY()
	TObjectPtr<ACoopArenaAttackRing> AttackRing;
};
