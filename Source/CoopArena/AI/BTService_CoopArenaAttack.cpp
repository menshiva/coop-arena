#include "BTService_CoopArenaAttack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "AbilitySystem/Abilities/CoopArenaGameplayAbility_EnemyAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTService_CoopArenaAttack::UBTService_CoopArenaAttack() {
	NodeName = "Attack";

	Interval = 0.1f;
	RandomDeviation = 0.05f;

	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CoopArenaAttack, BlackboardKey), AActor::StaticClass());
}

void UBTService_CoopArenaAttack::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, const float DeltaSeconds) {
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const auto ControllerPtr = OwnerComp.GetAIOwner();
	const auto BlackboardPtr = OwnerComp.GetBlackboardComponent();
	if (!ControllerPtr || !BlackboardPtr)
		return;

	const auto EnemyPtr = Cast<ACharacter>(ControllerPtr->GetPawn().Get());
	const auto PlayerPtr = Cast<AActor>(BlackboardPtr->GetValue<UBlackboardKeyType_Object>(BlackboardKey.GetSelectedKeyID()));
	if (!EnemyPtr || !PlayerPtr || !EnemyPtr->GetCharacterMovement()->IsMovingOnGround())
		return;

	const auto AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(EnemyPtr);
	if (!AbilitySystem)
		return;

	TArray<FGameplayAbilitySpec*> Specs;
	AbilitySystem->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(CoopArena_Ability_Attack_Enemy_Basic), Specs, false);
	// mid-swing TryActivateAbility would restart it
	if (Specs.IsEmpty() || Specs[0]->IsActive())
		return;

	// cooldown and Status.Dead are checked by TryActivateAbility
	const auto Attack = Cast<UCoopArenaGameplayAbility_EnemyAttack>(Specs[0]->GetPrimaryInstance());
	if (Attack && Attack->WouldHit(*PlayerPtr))
		AbilitySystem->TryActivateAbility(Specs[0]->Handle);
}
