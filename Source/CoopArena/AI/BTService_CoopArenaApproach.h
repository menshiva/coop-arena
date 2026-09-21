#pragma once

#include "BehaviorTree/BTService.h"
#include "BTService_CoopArenaApproach.generated.h"

class UNavigationSystemV1;
class ACoopArenaEnemyCharacter;

UCLASS()
class UBTService_CoopArenaApproach : public UBTService {
	GENERATED_BODY()
public:
	UBTService_CoopArenaApproach();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	struct FMemory {
		float StalledFor = 0.0f;
		float RepackingFor = 0.0f;
		uint32 RepackingDeathCount = 0;

		uint8 OffNavmeshTicks = 0;
	};

	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FMemory); }
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	static bool HandleOffNavmesh(const UNavigationSystemV1* NavSystem, ACoopArenaEnemyCharacter& Enemy, FMemory& Memory);

	bool IsArrived(ACoopArenaEnemyCharacter& Enemy, const AActor& Player, FMemory& Memory, float DeltaSeconds) const;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector ApproachLocationKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector ArrivedKey;

	UPROPERTY(EditAnywhere, Category="Arrival", meta=(ClampMin=0))
	float TouchPackingGapDistance = 75.0f;

	UPROPERTY(EditAnywhere, Category="Arrival", meta=(ClampMin=0))
	float SpeedConsideredAsStall = 50.0f;

	UPROPERTY(EditAnywhere, Category="Arrival", meta=(ClampMin=0))
	float TimeConsideredAsStall = 0.5f;

	UPROPERTY(EditAnywhere, Category="Arrival", meta=(ClampMin=0))
	float TimeToRepackAfterDeath = 0.5f;
};
