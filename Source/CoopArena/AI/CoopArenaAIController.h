#pragma once

#include "AIController.h"
#include "CoopArenaAIController.generated.h"

class UBehaviorTree;

UCLASS(Abstract)
class ACoopArenaAIController : public AAIController {
	GENERATED_BODY()
protected:
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(EditDefaultsOnly, Category="AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;
private:
	UFUNCTION()
	void OnLanded(const FHitResult& Hit);
};
