#include "CoopArenaAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/CrowdFollowingComponent.h"

static const FName TargetActorKey(TEXT("TargetActor"));

void ACoopArenaAIController::SetCrowdSeparation(const bool bEnable) const {
	if (const auto CrowdPtr = Cast<UCrowdFollowingComponent>(GetPathFollowingComponent()))
		CrowdPtr->SetCrowdSeparation(bEnable);
}

void ACoopArenaAIController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);

	if (const auto CharacterPtr = Cast<ACharacter>(InPawn))
		CharacterPtr->LandedDelegate.AddDynamic(this, &ACoopArenaAIController::OnLanded);

	if (const auto CrowdPtr = Cast<UCrowdFollowingComponent>(GetPathFollowingComponent()))
		CrowdPtr->SetCrowdSeparationWeight(CrowdSeparationWeight);
	SetCrowdSeparation(true);
}

void ACoopArenaAIController::OnLanded(const FHitResult&) {
	if (RunBehaviorTree(BehaviorTree))
		if (Blackboard)
			Blackboard->SetValueAsObject(TargetActorKey, UGameplayStatics::GetPlayerPawn(this, 0));
}
