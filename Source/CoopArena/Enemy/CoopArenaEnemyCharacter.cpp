#include "CoopArenaEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/CoopArenaAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/CoopArenaGameState.h"
#include "GameFramework/CharacterMovementComponent.h"
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
	BaseSpeed = FMath::RandRange(BaseSpeedRange.Min, BaseSpeedRange.Max);
}

#if WITH_EDITOR
void ACoopArenaEnemyCharacter::PostEditImport() {
	Super::PostEditImport();
	MaxHealth = FMath::RandRange(MaxHealthRange.Min, MaxHealthRange.Max);
	BaseSpeed = FMath::RandRange(BaseSpeedRange.Min, BaseSpeedRange.Max);
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

	HealthWidget->RequestRenderUpdate();
}

void ACoopArenaEnemyCharacter::Tick(const float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (const auto Camera = UGameplayStatics::GetPlayerCameraManager(this, 0))
		HealthWidget->SetWorldRotation((Camera->GetCameraLocation() - HealthWidget->GetComponentLocation()).Rotation());

	if (const auto PlayerPtr = UGameplayStatics::GetPlayerPawn(this, 0)) {
		// far from the player - faster, close - slower
		const float Distance = FVector::Dist2D(GetActorLocation(), PlayerPtr->GetActorLocation());
		const float Alpha = FMath::Clamp(FMath::GetRangePct(SpeedDistanceRange.Min, SpeedDistanceRange.Max, Distance), 0.0f, 1.0f);
		GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * (1.0f + SpeedPercentByDistanceRange.Interpolate(Alpha));
	}
}

bool ACoopArenaEnemyCharacter::IsArrivedToStandingPlayer() const {
	return bArrivedToStandingPlayer && EnemyDeathCountWhenArrived == ACoopArenaGameState::GetEnemyDeathCount(GetWorld());
}

bool ACoopArenaEnemyCharacter::IsRepackingAfterDeath() const {
	return bArrivedToStandingPlayer && EnemyDeathCountWhenArrived != ACoopArenaGameState::GetEnemyDeathCount(GetWorld());
}

void ACoopArenaEnemyCharacter::SetArrivedToStandingPlayer(const bool bValue) {
	bArrivedToStandingPlayer = bValue;
	EnemyDeathCountWhenArrived = ACoopArenaGameState::GetEnemyDeathCount(GetWorld());
}

void ACoopArenaEnemyCharacter::OnHealthChanged(const FOnAttributeChangeData&) const {
	if (const auto Widget = Cast<UCoopArenaHealthWidget>(HealthWidget->GetWidget())) {
		Widget->OnHealthChanged(FMath::RoundToInt(Attributes->GetHealth()), FMath::RoundToInt(Attributes->GetMaxHealth()));
		HealthWidget->RequestRenderUpdate();
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

	// only a death inside the standing crowd leaves a gap to re-pack into
	if (bArrivedToStandingPlayer)
		if (const auto GameState = GetWorld()->GetGameState<ACoopArenaGameState>())
			GameState->NotifyEnemyDeath();

	Destroy();
}
