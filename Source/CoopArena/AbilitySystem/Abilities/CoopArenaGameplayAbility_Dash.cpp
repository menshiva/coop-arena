#include "CoopArenaGameplayAbility_Dash.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
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
		const auto MovementPtr = CharacterPtr->GetCharacterMovement();
		if (!MovementPtr->Velocity.IsNearlyZero()) {
			if (CommitAbility(Handle, ActorInfo, ActivationInfo)) {
				auto Direction = CharacterPtr->GetLastMovementInputVector();
				if (Direction.IsNearlyZero())
					Direction = CharacterPtr->GetActorForwardVector();

				if (ActorInfo->AbilitySystemComponent.IsValid())
					ActorInfo->AbilitySystemComponent->PlayMontage(this, ActivationInfo, DashMontage, 1.0f);

				const auto Task = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
					this, FName("Dash"), Direction.GetSafeNormal2D(),
					MovementPtr->IsFalling() ? FlyingStrength : Strength, Duration, false,
					nullptr, ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity,
					FVector::ZeroVector, MovementPtr->MaxWalkSpeed, false
				);
				Task->OnFinish.AddDynamic(this, &UCoopArenaGameplayAbility_Dash::K2_EndAbility);
				Task->ReadyForActivation();

				return;
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}
