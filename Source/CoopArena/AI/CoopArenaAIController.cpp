#include "CoopArenaAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

static const FName TargetActorKey(TEXT("TargetActor"));

void ACoopArenaAIController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);

	if (const auto CharacterPtr = Cast<ACharacter>(InPawn))
		CharacterPtr->LandedDelegate.AddDynamic(this, &ACoopArenaAIController::OnLanded);
}

void ACoopArenaAIController::OnLanded(const FHitResult&) {
	if (RunBehaviorTree(BehaviorTree))
		if (Blackboard)
			Blackboard->SetValueAsObject(TargetActorKey, UGameplayStatics::GetPlayerPawn(this, 0));
}
