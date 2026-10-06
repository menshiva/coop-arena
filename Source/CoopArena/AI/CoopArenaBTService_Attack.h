#pragma once

#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "CoopArenaBTService_Attack.generated.h"

UCLASS()
class UCoopArenaBTService_Attack : public UBTService_BlackboardBase {
	GENERATED_BODY()
public:
	UCoopArenaBTService_Attack();
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
