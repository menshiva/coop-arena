#include "CoopArenaCharacter.h"
#include "AbilitySystemComponent.h"
#include "CoopArena.h"
#include "CoopArenaPlayerState.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "AbilitySystem/Abilities/CoopArenaGameplayAbility.h"
#include "AI/CoopArenaAttackRing.h"

ACoopArenaCharacter::ACoopArenaCharacter() {
	GetCapsuleComponent()->InitCapsuleSize(32.f, 90.0f);

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
	// FollowCamera->bUsePawnControlRotation = false;
}

void ACoopArenaCharacter::BeginPlay() {
	Super::BeginPlay();

	if (AttackRingClass) {
		FActorSpawnParameters Params;
		Params.Owner = this;
		AttackRing = GetWorld()->SpawnActor<ACoopArenaAttackRing>(AttackRingClass, GetActorLocation(), FRotator::ZeroRotator, Params);
	}
}

void ACoopArenaCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	if (AttackRing)
		AttackRing->Destroy();

	Super::EndPlay(EndPlayReason);
}

void ACoopArenaCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	if (const auto StatePtr = GetPlayerState<ACoopArenaPlayerState>()) {
		AbilitySystem = StatePtr->GetAbilitySystemComponent();
		AbilitySystem->InitAbilityActorInfo(StatePtr, this);

		if (InitStatsEffect) {
			auto Context = AbilitySystem->MakeEffectContext();
			Context.AddSourceObject(this);
			AbilitySystem->ApplyGameplayEffectToSelf(InitStatsEffect->GetDefaultObject<UGameplayEffect>(), 1.0f, Context);
		}

		for (const auto& Binding : AbilityBindings)
			if (Binding.bGrantedAtStart && Binding.AbilityClass && !AbilitySystem->FindAbilitySpecFromClass(Binding.AbilityClass))
				AbilitySystem->GiveAbility(FGameplayAbilitySpec(Binding.AbilityClass, 1));
	}
}

void ACoopArenaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	if (const auto EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACoopArenaCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ACoopArenaCharacter::Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACoopArenaCharacter::Look);

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Abilities
		for (const auto& Binding : AbilityBindings)
			if (Binding.InputAction)
				EnhancedInputComponent->BindAction(Binding.InputAction, ETriggerEvent::Started, this, &ACoopArenaCharacter::OnAbilityInput, Binding.AbilityTag);
	}
	else {
		UE_LOG(LogCoopArena, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACoopArenaCharacter::Move(const FInputActionValue& Value) {
	if (const auto ControllerPtr = GetController()) {
		const auto MovementVector = Value.Get<FVector2D>();

		// find out which way is forward
		const auto Rotation = ControllerPtr->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

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
