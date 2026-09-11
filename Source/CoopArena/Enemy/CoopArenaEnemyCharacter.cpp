#include "CoopArenaEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/CoopArenaAttributeSet.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetSystemLibrary.h"

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
	Attributes->OnDeath.AddUObject(this, &ACoopArenaEnemyCharacter::OnDeath);
}

void ACoopArenaEnemyCharacter::OnDeath(AActor* Killer) {
	// handle 2 balls at the same frame when dead
	Attributes->OnDeath.RemoveAll(this);

	if (const auto KillerPlayerState = Cast<APlayerState>(Killer)) {
		KillerPlayerState->SetScore(KillerPlayerState->GetScore() + 1.0f);
		UKismetSystemLibrary::PrintString(
			this, FString::Printf(TEXT("%s: %.0f kills"), *KillerPlayerState->GetPlayerName(), KillerPlayerState->GetScore()),
			true, true, FLinearColor::Green, 5.0f
		);
	}

	Destroy();
}
