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

		// which side of the player the arc goes: true - his right, false - his left
		bool bRightOrbitSide = true;

		// ticks in a row on the ground off the navmesh
		uint8 OffNavmeshTicks = 0;
	};
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FMemory); }
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	static bool HandleOffNavmesh(const UNavigationSystemV1* NavSystem, ACoopArenaEnemyCharacter& Enemy, FMemory& Memory);

	TOptional<FVector> ChasePoint(const UNavigationSystemV1* NavSystem, ACoopArenaEnemyCharacter& Enemy, const AActor& Player, FMemory& Memory) const;

	bool IsArrived(ACoopArenaEnemyCharacter& Enemy, const AActor& Player, FMemory& Memory, float DeltaSeconds) const;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector ApproachLocationKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector ArrivedKey;

	// bearing of the enemy in the player's frame of motion: 0 - ahead of him, 90 - at his side, 180 - behind;
	// behind this one an interceptor starts going around the player
	UPROPERTY(EditAnywhere, Category="Chase", meta=(ClampMin=0, ClampMax=180))
	float ArcStartBearing = 110.0f;

	// where the arc ends and the attack starts (for intercept role)
	UPROPERTY(EditAnywhere, Category="Chase", meta=(ClampMin=0, ClampMax=180))
	float InterceptArcEndBearing = 70.0f;

	// where the arc ends and the attack starts (for counter run role)
	UPROPERTY(EditAnywhere, Category="Chase", meta=(ClampMin=0, ClampMax=180))
	float CounterRunArcEndBearing = 20.0f;

	// how far along the arc the next point is put
	UPROPERTY(EditAnywhere, Category="Chase", meta=(ClampMin=0))
	float ArcStepDistance = 400.0f;

	// the arc point is projected to the navmesh with this extent
	UPROPERTY(EditAnywhere, Category="Chase")
	FVector ProjectionExtent = FVector(100.0, 100.0, 250.0);

	UPROPERTY(EditAnywhere, Category="Arrival", meta=(ClampMin=0))
	float TouchPackingGapDistance = 75.0f;

	UPROPERTY(EditAnywhere, Category="Arrival", meta=(ClampMin=0))
	float SpeedConsideredAsStall = 50.0f;

	UPROPERTY(EditAnywhere, Category="Arrival", meta=(ClampMin=0))
	float TimeConsideredAsStall = 0.5f;

	UPROPERTY(EditAnywhere, Category="Arrival", meta=(ClampMin=0))
	float TimeToRepackAfterDeath = 0.5f;
};
