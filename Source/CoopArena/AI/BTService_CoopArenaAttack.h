#pragma once

#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_CoopArenaAttack.generated.h"

UCLASS()
class UBTService_CoopArenaAttack : public UBTService_BlackboardBase {
	GENERATED_BODY()
public:
	UBTService_CoopArenaAttack();
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
