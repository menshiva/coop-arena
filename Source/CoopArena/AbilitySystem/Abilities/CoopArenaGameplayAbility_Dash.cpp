#include "CoopArenaGameplayAbility_Dash.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UCoopArenaGameplayAbility_Dash::UCoopArenaGameplayAbility_Dash() {
	FGameplayTagContainer Tags;
	Tags.AddTagFast(CoopArena_Ability_Dash);
	SetAssetTags(Tags);
}

void UCoopArenaGameplayAbility_Dash::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData
) {
	if (CommitAbility(Handle, ActorInfo, ActivationInfo)) {
		if (const auto CharacterPtr = Cast<ACharacter>(ActorInfo->AvatarActor.Get())) {
			auto Direction = CharacterPtr->GetLastMovementInputVector();
			if (Direction.IsNearlyZero())
				Direction = CharacterPtr->GetActorForwardVector();

			if (DashMontage) {
				const auto MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
					this, NAME_None, DashMontage, DashMontage->CalculateSequenceLength() / Duration,
					NAME_None, false
				);
				MontageTask->ReadyForActivation();
			}

			const auto MovementPtr = CharacterPtr->GetCharacterMovement();
			const auto FinishMode = MovementPtr->IsFalling()
				? ERootMotionFinishVelocityMode::SetVelocity
				: ERootMotionFinishVelocityMode::ClampVelocity;
			const auto Task = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
				this, FName("Dash"), Direction.GetSafeNormal2D(), Strength, Duration,
				false, nullptr, FinishMode, FVector::ZeroVector,
				MovementPtr->MaxWalkSpeed, false
			);
			Task->OnFinish.AddDynamic(this, &UCoopArenaGameplayAbility_Dash::K2_EndAbility);
			Task->ReadyForActivation();
			return;
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}
