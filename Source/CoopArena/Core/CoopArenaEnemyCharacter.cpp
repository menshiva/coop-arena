#include "CoopArenaEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/CoopArenaAttributeSet.h"

ACoopArenaEnemyCharacter::ACoopArenaEnemyCharacter() {
	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	Attributes = CreateDefaultSubobject<UCoopArenaAttributeSet>(TEXT("Attributes"));
}

void ACoopArenaEnemyCharacter::PostActorCreated() {
	Super::PostActorCreated();
	MaxHealth = FMath::FRandRange(MaxHealthRange.Min, MaxHealthRange.Max);
}

#if WITH_EDITOR
void ACoopArenaEnemyCharacter::PostEditImport() {
	Super::PostEditImport();
	MaxHealth = FMath::FRandRange(MaxHealthRange.Min, MaxHealthRange.Max);
}
#endif

void ACoopArenaEnemyCharacter::BeginPlay() {
	Super::BeginPlay();

	AbilitySystem->InitAbilityActorInfo(this, this);

	Attributes->SetMaxHealth(MaxHealth);
	Attributes->SetHealth(MaxHealth);
}
