#include "CoopArenaAnimNotify_SendGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"

void UCoopArenaAnimNotify_SendGameplayEvent::Notify(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference
) {
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp && EventTag.IsValid())
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag, FGameplayEventData());
}

FString UCoopArenaAnimNotify_SendGameplayEvent::GetNotifyName_Implementation() const {
	return EventTag.IsValid() ? EventTag.ToString() : Super::GetNotifyName_Implementation();
}
