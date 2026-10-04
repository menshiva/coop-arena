#include "CoopArenaCharacter.h"
#include "CoopArenaPlayerState.h"
#include "EnhancedInputComponent.h"
#include "AbilitySystem/Abilities/CoopArenaGameplayAbility.h"
#include "AbilitySystem/CoopArenaAttributeSet.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"

ACoopArenaCharacter::ACoopArenaCharacter() {
	GetCapsuleComponent()->InitCapsuleSize(32.0f, 90.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	// bUseControllerRotationPitch = false;
	// bUseControllerRotationYaw = false;
	// bUseControllerRotationRoll = false;

	// Configure character movement
	// GetCharacterMovement()->bOrientRotationToMovement = true;
	// GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	// CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
}

void ACoopArenaCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);

	if (const auto StatePtr = GetPlayerState<ACoopArenaPlayerState>()) {
		AbilitySystem = StatePtr->GetAbilitySystemComponent();
		AbilitySystem->InitAbilityActorInfo(StatePtr, this);

		for (const auto& Effect : {InitStatsEffect, SpawnProtectionEffect})
			if (Effect)
				AbilitySystem->ApplyGameplayEffectToSelf(Effect.GetDefaultObject(), 1.0f, AbilitySystem->MakeEffectContext());

		for (const auto& Binding : AbilityBindings)
			if (Binding.bGrantedAtStart && Binding.AbilityClass && !AbilitySystem->FindAbilitySpecFromClass(Binding.AbilityClass))
				AbilitySystem->GiveAbility(FGameplayAbilitySpec(Binding.AbilityClass, 1));

		StatePtr->GetAttributes()->OnDeath.AddUObject(this, &ACoopArenaCharacter::OnDeath);
	}
}

void ACoopArenaCharacter::UnPossessed() {
	if (const auto StatePtr = GetPlayerState<ACoopArenaPlayerState>()) {
		StatePtr->GetAttributes()->OnDeath.RemoveAll(this);
		StatePtr->GetAbilitySystemComponent()->CancelAllAbilities();
	}

	Super::UnPossessed();
}

void ACoopArenaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	if (const auto EnhancedInputComponentPtr = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		// Moving
		EnhancedInputComponentPtr->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACoopArenaCharacter::Move);
		EnhancedInputComponentPtr->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACoopArenaCharacter::Look);

		// Jumping
		EnhancedInputComponentPtr->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponentPtr->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Abilities
		for (const auto& Binding : AbilityBindings)
			if (Binding.InputAction)
				EnhancedInputComponentPtr->BindAction(Binding.InputAction, ETriggerEvent::Started, this, &ACoopArenaCharacter::OnAbilityInput, Binding.AbilityTag);
	}
}

void ACoopArenaCharacter::Move(const FInputActionValue& Value) {
	if (const auto ControllerPtr = GetController()) {
		const auto MovementVector = Value.Get<FVector2D>();

		// find out which way is forward
		const auto Rotation = ControllerPtr->GetControlRotation();
		const FRotator YawRotation(0.0, Rotation.Yaw, 0.0);

		const auto ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const auto RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ACoopArenaCharacter::Look(const FInputActionValue& Value) {
	if (GetController()) {
		const auto LookAxisVector = Value.Get<FVector2D>();
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACoopArenaCharacter::OnAbilityInput(const FGameplayTag AbilityTag) {
	if (AbilitySystem.IsValid())
		AbilitySystem->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));
}

void ACoopArenaCharacter::OnDeath(AActor*) {
	Destroy();
}
