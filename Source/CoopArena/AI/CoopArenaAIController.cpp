#include "CoopArenaAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/CoopArenaEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/CrowdFollowingComponent.h"

static const FName TargetActorKey(TEXT("TargetActor"));

void ACoopArenaAIController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);

	if (const auto CharacterPtr = Cast<ACharacter>(InPawn))
		CharacterPtr->LandedDelegate.AddDynamic(this, &ACoopArenaAIController::OnLanded);

	if (const auto PlayerControllerPtr = UGameplayStatics::GetPlayerController(this, 0))
		PlayerControllerPtr->OnPossessedPawnChanged.AddUniqueDynamic(this, &ACoopArenaAIController::OnPlayerPawnChanged);

	if (const auto CrowdPtr = Cast<UCrowdFollowingComponent>(GetPathFollowingComponent()))
		CrowdPtr->SetCrowdSlowdownAtGoal(false);
}

void ACoopArenaAIController::OnLanded(const FHitResult&) {
	if (RunBehaviorTree(BehaviorTree))
		if (Blackboard)
			Blackboard->SetValueAsObject(TargetActorKey, UGameplayStatics::GetPlayerPawn(this, 0));
}

void ACoopArenaAIController::OnPlayerPawnChanged(APawn*, APawn* NewPawn) const {
	if (!NewPawn || !Blackboard)
		return; // the old pawn is gone, or not landed yet

	// reset target
	Blackboard->SetValueAsObject(TargetActorKey, NewPawn);
	if (const auto EnemyPtr = GetPawn<ACoopArenaEnemyCharacter>())
		EnemyPtr->SetArrivedToStandingPlayer(false);
}
