#pragma once

#include "BehaviorTree/BTService.h"
#include "BTService_AttackSlot.generated.h"

UCLASS()
class UBTService_AttackSlot : public UBTService {
	GENERATED_BODY()
public:
	UBTService_AttackSlot();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector AttackSlotKey;
private:
	static void SetEngaged(AAIController& Controller, AActor* Target);
};
