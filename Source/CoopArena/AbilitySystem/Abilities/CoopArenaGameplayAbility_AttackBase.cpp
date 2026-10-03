#include "CoopArenaGameplayAbility_AttackBase.h"
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

	const auto EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CoopArena_Event_Attack);
	EventTask->EventReceived.AddDynamic(this, &UCoopArenaGameplayAbility_AttackBase::OnAttackEvent);
	EventTask->ReadyForActivation();

	const auto MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage);
	MontageTask->OnCompleted.AddDynamic(this, &UCoopArenaGameplayAbility_AttackBase::K2_EndAbility);
	MontageTask->OnBlendOut.AddDynamic(this, &UCoopArenaGameplayAbility_AttackBase::K2_EndAbility);
	MontageTask->OnInterrupted.AddDynamic(this, &UCoopArenaGameplayAbility_AttackBase::K2_EndAbility);
	MontageTask->OnCancelled.AddDynamic(this, &UCoopArenaGameplayAbility_AttackBase::K2_EndAbility);
	MontageTask->ReadyForActivation();
}
