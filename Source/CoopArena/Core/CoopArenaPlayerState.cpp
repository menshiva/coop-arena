#include "CoopArenaPlayerState.h"
#include "AbilitySystem/CoopArenaAttributeSet.h"

ACoopArenaPlayerState::ACoopArenaPlayerState() {
	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	Attributes = CreateDefaultSubobject<UCoopArenaAttributeSet>(TEXT("Attributes"));
}
