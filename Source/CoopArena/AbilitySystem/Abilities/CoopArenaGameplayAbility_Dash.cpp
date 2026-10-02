#include "CoopArenaGameplayAbility_Dash.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UCoopArenaGameplayAbility_Dash::UCoopArenaGameplayAbility_Dash() {
	FGameplayTagContainer Tags;
	Tags.AddTagFast(CoopArena_Ability_Dash);
	SetAssetTags(Tags);
}

void UCoopArenaGameplayAbility_Dash::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData*
) {
	if (const auto CharacterPtr = Cast<ACharacter>(ActorInfo->AvatarActor.Get())) {
		if (CommitAbility(Handle, ActorInfo, ActivationInfo)) {
			auto Direction = CharacterPtr->GetLastMovementInputVector();
			if (Direction.IsNearlyZero())
				Direction = CharacterPtr->GetActorForwardVector();
			DashDirection = Direction.GetSafeNormal2D();

			if (ActorInfo->AbilitySystemComponent.IsValid())
				ActorInfo->AbilitySystemComponent->PlayMontage(this, ActivationInfo, DashMontage, 1.0f);

			{
				// pass through enemies
				PushedActorsSet.Reset();
				CharacterPtr->OnCharacterMovementUpdated.AddDynamic(this, &UCoopArenaGameplayAbility_Dash::PushTouched);
				CharacterPtr->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
			}

			{
				// run task
				const auto MovementPtr = CharacterPtr->GetCharacterMovement();
				const auto Task = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
					this, FName("Dash"), DashDirection,
					MovementPtr->IsFalling() ? FlyingStrength : Strength, Duration, false,
					nullptr, ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity,
					FVector::ZeroVector, MovementPtr->MaxWalkSpeed, false
				);
				Task->OnFinish.AddDynamic(this, &UCoopArenaGameplayAbility_Dash::K2_EndAbility);
				Task->ReadyForActivation();
			}

			return;
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UCoopArenaGameplayAbility_Dash::EndAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const bool bReplicateEndAbility, const bool bWasCancelled
) {
	if (const auto CharacterPtr = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr) {
		// revert pass through enemies
		CharacterPtr->OnCharacterMovementUpdated.RemoveDynamic(this, &UCoopArenaGameplayAbility_Dash::PushTouched);
		CharacterPtr->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// ReSharper disable once CppPassValueParameterByConstReference
void UCoopArenaGameplayAbility_Dash::PushTouched(float, const FVector OldLocation, FVector) {
	const auto CharacterPtr = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!CharacterPtr)
		return;

	const auto CapsulePtr = CharacterPtr->GetCapsuleComponent();
	TArray<FHitResult> Hits;
	CharacterPtr->GetWorld()->SweepMultiByObjectType(
		Hits, OldLocation, CharacterPtr->GetActorLocation(), CapsulePtr->GetComponentQuat(), FCollisionObjectQueryParams(ECC_Pawn),
		CapsulePtr->GetCollisionShape(), FCollisionQueryParams(NAME_None, false, CharacterPtr)
	);
	for (const auto& Hit : Hits) {
		const auto OtherPtr = Cast<ACharacter>(Hit.GetActor());
		if (!OtherPtr)
			continue;

		bool AlreadyInSet = false;
		PushedActorsSet.Add(OtherPtr, &AlreadyInSet);
		if (AlreadyInSet)
			continue;

		{
			// launch enemy to the side
			auto Side = FVector::VectorPlaneProject(OtherPtr->GetActorLocation() - OldLocation, DashDirection).GetSafeNormal2D();
			if (Side.IsZero())
				Side = FVector::CrossProduct(FVector::UpVector, DashDirection);
			OtherPtr->LaunchCharacter(Side * PushStrength + FVector::UpVector * PushUpStrength, true, true);
		}
	}
}
