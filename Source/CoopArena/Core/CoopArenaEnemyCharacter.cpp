#include "CoopArenaEnemyCharacter.h"

void ACoopArenaEnemyCharacter::PostActorCreated() {
	Super::PostActorCreated();
	SetRandomHealth();
}

#if WITH_EDITOR
void ACoopArenaEnemyCharacter::PostEditImport() {
	Super::PostEditImport();
	SetRandomHealth();
}
#endif

void ACoopArenaEnemyCharacter::SetRandomHealth() {
	Health = FMath::FRandRange(HealthRange.Min, HealthRange.Max);
}
