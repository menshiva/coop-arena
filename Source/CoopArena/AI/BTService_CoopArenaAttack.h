#pragma once

#include "BehaviorTree/BTService.h"
#include "BTService_CoopArenaAttack.generated.h"

UCLASS()
class UBTService_CoopArenaAttack : public UBTService {
	GENERATED_BODY()
public:
	UBTService_CoopArenaAttack();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category="Attack", meta=(Categories="CoopArena.Ability"))
	FGameplayTag AttackAbilityTag;
};
