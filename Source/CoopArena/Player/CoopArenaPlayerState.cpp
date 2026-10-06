#include "CoopArenaPlayerState.h"
#include "AbilitySystem/Attributes/CoopArenaAttributeSet.h"

ACoopArenaPlayerState::ACoopArenaPlayerState() {
	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	Attributes = CreateDefaultSubobject<UCoopArenaAttributeSet>(TEXT("Attributes"));
}
