#include "BTService_AttackSlot.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CoopArenaAIController.h"
#include "CoopArenaAttackRing.h"
#include "Core/CoopArenaCharacter.h"

UBTService_AttackSlot::UBTService_AttackSlot() {
	NodeName = "AttackSlot";
	INIT_SERVICE_NODE_NOTIFY_FLAGS();
	Interval = 0.25f;

	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_AttackSlot, TargetActorKey), AActor::StaticClass());
	AttackSlotKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_AttackSlot, AttackSlotKey));
}

void UBTService_AttackSlot::InitializeFromAsset(UBehaviorTree& Asset) {
	Super::InitializeFromAsset(Asset);

	if (const auto BlackboardPtr = GetBlackboardAsset()) {
		TargetActorKey.ResolveSelectedKey(*BlackboardPtr);
		AttackSlotKey.ResolveSelectedKey(*BlackboardPtr);
	}
}

void UBTService_AttackSlot::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, const float DeltaSeconds) {
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const auto BlackboardPtr = OwnerComp.GetBlackboardComponent();
	if (!BlackboardPtr)
		return;

	const auto ControllerPtr = OwnerComp.GetAIOwner();
	if (!ControllerPtr)
		return;

	const auto TargetPtr = Cast<ACoopArenaCharacter>(BlackboardPtr->GetValue<UBlackboardKeyType_Object>(TargetActorKey.GetSelectedKeyID()));
	if (!TargetPtr)
		return;
	const auto RingPtr = TargetPtr->GetAttackRing();
	if (!RingPtr)
		return;

	const auto KeyID = AttackSlotKey.GetSelectedKeyID();
	const bool bWasEngaged = BlackboardPtr->IsVectorValueSet(KeyID);

	FVector Location;
	if (RingPtr->Acquire(*ControllerPtr, Location)) {
		// the same value does not notify observers, a new point restarts the move
		BlackboardPtr->SetValue<UBlackboardKeyType_Vector>(KeyID, Location);
		if (!bWasEngaged)
			SetEngaged(*ControllerPtr, TargetPtr);
	}
	else if (bWasEngaged) {
		BlackboardPtr->ClearValue(KeyID);
		SetEngaged(*ControllerPtr, nullptr);
	}
}

void UBTService_AttackSlot::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	const auto BlackboardPtr = OwnerComp.GetBlackboardComponent();
	const auto ControllerPtr = OwnerComp.GetAIOwner();
	if (!BlackboardPtr || !ControllerPtr)
		return;

	if (const auto TargetPtr = Cast<ACoopArenaCharacter>(BlackboardPtr->GetValue<UBlackboardKeyType_Object>(TargetActorKey.GetSelectedKeyID())))
		if (const auto RingPtr = TargetPtr->GetAttackRing())
			RingPtr->Release(*ControllerPtr);

	const auto KeyID = AttackSlotKey.GetSelectedKeyID();
	if (BlackboardPtr->IsVectorValueSet(KeyID)) {
		BlackboardPtr->ClearValue(KeyID);
		SetEngaged(*ControllerPtr, nullptr);
	}
}

// Engaged: face the target even while walking and no crowd separation
void UBTService_AttackSlot::SetEngaged(AAIController& Controller, AActor* Target) {
	if (Target)
		Controller.SetFocus(Target, EAIFocusPriority::Gameplay);
	else
		Controller.ClearFocus(EAIFocusPriority::Gameplay);

	if (const auto OwnPtr = Cast<ACoopArenaAIController>(&Controller))
		OwnPtr->SetCrowdSeparation(!Target);
}
