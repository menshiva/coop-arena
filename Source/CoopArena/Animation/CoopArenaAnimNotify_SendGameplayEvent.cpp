#include "CoopArenaAnimNotify_SendGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/SkeletalMeshComponent.h"

void UCoopArenaAnimNotify_SendGameplayEvent::Notify(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference
) {
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !EventTag.IsValid())
		return;

	const auto OwnerPtr = MeshComp->GetOwner();
	if (!OwnerPtr)
		return;

	FGameplayEventData EventData;
	EventData.EventTag = EventTag;
	EventData.Instigator = OwnerPtr;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerPtr, EventTag, EventData);
}

FString UCoopArenaAnimNotify_SendGameplayEvent::GetNotifyName_Implementation() const {
	return EventTag.IsValid() ? EventTag.ToString() : Super::GetNotifyName_Implementation();
}
