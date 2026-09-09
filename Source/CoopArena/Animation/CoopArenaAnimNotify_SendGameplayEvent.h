#pragma once

#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CoopArenaAnimNotify_SendGameplayEvent.generated.h"

UCLASS()
class UCoopArenaAnimNotify_SendGameplayEvent : public UAnimNotify {
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
protected:
	UPROPERTY(EditAnywhere, meta=(Categories="CoopArena.Event"))
	FGameplayTag EventTag;
};
