#pragma once

#include "DetourCrowdAIController.h"
#include "CoopArenaAIController.generated.h"

class UBehaviorTree;

UCLASS(Abstract)
class ACoopArenaAIController : public ADetourCrowdAIController {
	GENERATED_BODY()
public:
	void SetCrowdSeparation(bool bEnable) const;
protected:
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(EditDefaultsOnly, Category="AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditDefaultsOnly, Category="AI")
	float CrowdSeparationWeight = 2.0f;
private:
	UFUNCTION()
	void OnLanded(const FHitResult& Hit);
};
