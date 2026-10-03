#include "CoopArenaGameplayAbility_EnemyAttack.h"
#include "AbilitySystemComponent.h"
#include "EngineUtils.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Algo/SelectRandomWeighted.h"
#include "Animation/CoopArenaAnimNotify_SendGameplayEvent.h"
#include "Core/CoopArenaCharacter.h"

UCoopArenaGameplayAbility_EnemyAttack::UCoopArenaGameplayAbility_EnemyAttack() {
	FGameplayTagContainer Tags;
	Tags.AddTagFast(CoopArena_Ability_Attack_Enemy_Basic);
	SetAssetTags(Tags);
}

void UCoopArenaGameplayAbility_EnemyAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData*
) {
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const auto EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CoopArena_Event_Attack);
	EventTask->EventReceived.AddDynamic(this, &UCoopArenaGameplayAbility_EnemyAttack::OnAttackEvent);
	EventTask->ReadyForActivation();

	const auto MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage);
	MontageTask->OnCompleted.AddDynamic(this, &UCoopArenaGameplayAbility_EnemyAttack::K2_EndAbility);
	MontageTask->OnBlendOut.AddDynamic(this, &UCoopArenaGameplayAbility_EnemyAttack::K2_EndAbility);
	MontageTask->OnInterrupted.AddDynamic(this, &UCoopArenaGameplayAbility_EnemyAttack::K2_EndAbility);
	MontageTask->OnCancelled.AddDynamic(this, &UCoopArenaGameplayAbility_EnemyAttack::K2_EndAbility);
	MontageTask->ReadyForActivation();
}

void UCoopArenaGameplayAbility_EnemyAttack::OnAttackEvent(FGameplayEventData) const {
	const auto EnemyPtr = GetAvatarActorFromActorInfo();
	const auto DamageSpec = MakeOutgoingGameplayEffectSpec(DamageEffect);
	if (!EnemyPtr || !DamageSpec.IsValid())
		return;

	const auto EnemyPos = EnemyPtr->GetActorLocation();
	const auto Forward = FVector2D(EnemyPtr->GetActorForwardVector());
	const float MinCos = FMath::Cos(FMath::DegreesToRadians(HitHalfAngle));

	for (TActorIterator<ACoopArenaCharacter> It(GetWorld()); It; ++It) {
		const auto ToPlayer = It->GetActorLocation() - EnemyPos;
		if (ToPlayer.SizeSquared() > FMath::Square(HitDistance))
			continue;
		if (FVector2D::DotProduct(Forward, FVector2D(ToPlayer).GetSafeNormal()) < MinCos)
			continue;

		if (const auto AbilitySystem = It->GetAbilitySystemComponent())
			AbilitySystem->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
	}
}

bool UCoopArenaGameplayAbility_EnemyAttack::WouldHit(const AActor& Target) const {
	const auto EnemyPtr = GetAvatarActorFromActorInfo();
	if (!EnemyPtr)
		return false;

	// where the target will be at the notify if both keep their velocities
	const auto ToTargetAtHit = Target.GetActorLocation() - EnemyPtr->GetActorLocation() + (Target.GetVelocity() - EnemyPtr->GetVelocity()) * GetHitTime();
	return ToTargetAtHit.SizeSquared() <= FMath::Square(HitDistance);
}

int32 UCoopArenaGameplayAbility_EnemyAttack::RollDamage() const {
	const auto Chosen = Algo::SelectRandomWeightedBy(DamageChances, [] (const TPair<int32, float>& P) { return P.Value; });
	return Chosen ? Chosen->Key : 1;
}

float UCoopArenaGameplayAbility_EnemyAttack::GetHitTime() const {
	if (AttackMontage)
		for (const auto& Event : AttackMontage->Notifies)
			if (const auto Notify = Cast<UCoopArenaAnimNotify_SendGameplayEvent>(Event.Notify))
				if (Notify->GetEventTag() == CoopArena_Event_Attack)
					return Event.GetTriggerTime();
	return 0.0f;
}
