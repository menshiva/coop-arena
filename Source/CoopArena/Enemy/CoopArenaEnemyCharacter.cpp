#include "CoopArenaEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/CoopArenaAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/CoopArenaHealthWidget.h"

ACoopArenaEnemyCharacter::ACoopArenaEnemyCharacter() {
	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	Attributes = CreateDefaultSubobject<UCoopArenaAttributeSet>(TEXT("Attributes"));

	HealthWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidget"));
	HealthWidget->SetupAttachment(GetCapsuleComponent());
	HealthWidget->CastShadow = false;
}

void ACoopArenaEnemyCharacter::PostActorCreated() {
	Super::PostActorCreated();
	MaxHealth = FMath::RandRange(MaxHealthRange.Min, MaxHealthRange.Max);
}

#if WITH_EDITOR
void ACoopArenaEnemyCharacter::PostEditImport() {
	Super::PostEditImport();
	MaxHealth = FMath::RandRange(MaxHealthRange.Min, MaxHealthRange.Max);
}
#endif

void ACoopArenaEnemyCharacter::BeginPlay() {
	Super::BeginPlay();

	AbilitySystem->InitAbilityActorInfo(this, this);

	AbilitySystem->GetGameplayAttributeValueChangeDelegate(UCoopArenaAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &ACoopArenaEnemyCharacter::OnHealthChanged);
	AbilitySystem->GetGameplayAttributeValueChangeDelegate(UCoopArenaAttributeSet::GetHealthAttribute()).AddUObject(this, &ACoopArenaEnemyCharacter::OnHealthChanged);

	Attributes->SetMaxHealth(MaxHealth);
	Attributes->SetHealth(MaxHealth);
	Attributes->OnDeath.AddUObject(this, &ACoopArenaEnemyCharacter::OnDeath);
}

void ACoopArenaEnemyCharacter::Tick(const float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (const auto Camera = UGameplayStatics::GetPlayerCameraManager(this, 0))
		HealthWidget->SetWorldRotation((Camera->GetCameraLocation() - HealthWidget->GetComponentLocation()).Rotation());
}

void ACoopArenaEnemyCharacter::OnHealthChanged(const FOnAttributeChangeData&) const {
	if (const auto Widget = Cast<UCoopArenaHealthWidget>(HealthWidget->GetWidget())) {
		Widget->OnHealthChanged(FMath::RoundToInt(Attributes->GetHealth()), FMath::RoundToInt(Attributes->GetMaxHealth()));
		HealthWidget->RequestRedraw();
	}
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
