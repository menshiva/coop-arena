#include "CoopArenaBTService_Attack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "CoopArenaGameplayTags.h"
#include "AbilitySystem/Abilities/CoopArenaGameplayAbility_EnemyAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UCoopArenaBTService_Attack::UCoopArenaBTService_Attack() {
	NodeName = "Attack";

	Interval = 0.1f;
	RandomDeviation = 0.05f;

	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UCoopArenaBTService_Attack, BlackboardKey), AActor::StaticClass());
}

void UCoopArenaBTService_Attack::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, const float DeltaSeconds) {
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const auto ControllerPtr = OwnerComp.GetAIOwner();
	const auto BlackboardPtr = OwnerComp.GetBlackboardComponent();
	if (!ControllerPtr || !BlackboardPtr)
		return;

	const auto EnemyPtr = ControllerPtr->GetPawn<ACharacter>();
	const auto PlayerPtr = Cast<AActor>(BlackboardPtr->GetValue<UBlackboardKeyType_Object>(BlackboardKey.GetSelectedKeyID()));
	if (!EnemyPtr || !PlayerPtr || !EnemyPtr->GetCharacterMovement()->IsMovingOnGround())
		return;

	const auto AbilitySystemPtr = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(EnemyPtr);
	if (!AbilitySystemPtr)
		return;

	TArray<FGameplayAbilitySpec*> Specs;
	AbilitySystemPtr->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(CoopArena_Ability_Attack_Enemy_Basic), Specs, false);
	if (Specs.IsEmpty() || Specs[0]->IsActive())
		return; // mid-swing TryActivateAbility would restart it

	// the cooldown is checked by TryActivateAbility
	const auto AttackPtr = Cast<UCoopArenaGameplayAbility_EnemyAttack>(Specs[0]->GetPrimaryInstance());
	if (AttackPtr && AttackPtr->WouldHit(*PlayerPtr))
		AbilitySystemPtr->TryActivateAbility(Specs[0]->Handle);
}
