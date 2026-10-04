#include "CoopArenaGameplayAbility_AttackBase.h"
#include "CoopArenaGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

void UCoopArenaGameplayAbility_AttackBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData*
) {
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	{
		// the attack itself goes on the montage notify
		const auto EventTaskPtr = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CoopArena_Event_Attack);
		EventTaskPtr->EventReceived.AddDynamic(this, &UCoopArenaGameplayAbility_AttackBase::OnAttackEvent);
		EventTaskPtr->ReadyForActivation();
	}

	{
		// swing; any end of the montage ends the ability
		const auto MontageTaskPtr = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage);
		MontageTaskPtr->OnCompleted.AddDynamic(this, &UCoopArenaGameplayAbility_AttackBase::K2_EndAbility);
		MontageTaskPtr->OnBlendOut.AddDynamic(this, &UCoopArenaGameplayAbility_AttackBase::K2_EndAbility);
		MontageTaskPtr->OnInterrupted.AddDynamic(this, &UCoopArenaGameplayAbility_AttackBase::K2_EndAbility);
		MontageTaskPtr->OnCancelled.AddDynamic(this, &UCoopArenaGameplayAbility_AttackBase::K2_EndAbility);
		MontageTaskPtr->ReadyForActivation();
	}
}
