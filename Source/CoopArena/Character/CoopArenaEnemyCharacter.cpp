#include "CoopArenaEnemyCharacter.h"
#include "EngineUtils.h"
#include "AbilitySystem/Abilities/CoopArenaGameplayAbility_EnemyAttack.h"
#include "AbilitySystem/Attributes/CoopArenaAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameModes/CoopArenaEnemySpawner.h"
#include "GameModes/CoopArenaGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Projectiles/CoopArenaProjectileManager.h"
#include "UI/CoopArenaHealthWidget.h"

ACoopArenaEnemyCharacter::ACoopArenaEnemyCharacter() {
	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	Attributes = CreateDefaultSubobject<UCoopArenaAttributeSet>(TEXT("Attributes"));

	HealthWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidget"));
	HealthWidget->SetupAttachment(GetCapsuleComponent());
	HealthWidget->CastShadow = false;
	HealthWidget->SetTickMode(ETickMode::Disabled);
	HealthWidget->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
}

void ACoopArenaEnemyCharacter::PostActorCreated() {
	Super::PostActorCreated();

#if WITH_EDITOR
	// placed by hand
	if (const auto WorldPtr = GetWorld(); WorldPtr && !WorldPtr->IsGameWorld())
		RollStatsFromSpawner();
#endif
}

#if WITH_EDITOR
void ACoopArenaEnemyCharacter::PostEditImport() {
	Super::PostEditImport();

	RollStatsFromSpawner();
}
#endif

void ACoopArenaEnemyCharacter::BeginPlay() {
	Super::BeginPlay();

	AbilitySystem->InitAbilityActorInfo(this, this);
	if (AttackAbility)
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(AttackAbility, 1));

	AbilitySystem->GetGameplayAttributeValueChangeDelegate(UCoopArenaAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &ACoopArenaEnemyCharacter::OnHealthChanged);
	AbilitySystem->GetGameplayAttributeValueChangeDelegate(UCoopArenaAttributeSet::GetHealthAttribute()).AddUObject(this, &ACoopArenaEnemyCharacter::OnHealthChanged);

	Attributes->SetMaxHealth(Stats.MaxHealth);
	Attributes->SetHealth(Stats.MaxHealth);
	Attributes->OnDeath.AddUObject(this, &ACoopArenaEnemyCharacter::OnDeath);

	// flying balls don't block our movement
	if (const TActorIterator<ACoopArenaProjectileManager> It(GetWorld()); It)
		MoveIgnoreActorAdd(*It);
}

void ACoopArenaEnemyCharacter::Tick(const float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (const auto PlayerController0Ptr = UGameplayStatics::GetPlayerController(this, 0)) {
		if (PlayerController0Ptr->PlayerCameraManager) {
			const auto Dir = PlayerController0Ptr->PlayerCameraManager->GetCameraLocation() - HealthWidget->GetComponentLocation();
			HealthWidget->SetWorldRotation(Dir.Rotation());
		}

		if (const auto PlayerPtr = PlayerController0Ptr->GetCharacter())
			GetCharacterMovement()->MaxWalkSpeed = PlayerPtr->GetCharacterMovement()->MaxWalkSpeed * Stats.SpeedFactor * (bOvertaking ? Stats.OvertakeSpeedFactor : 1.0f);
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
	if (const auto WidgetPtr = Cast<UCoopArenaHealthWidget>(HealthWidget->GetWidget())) {
		WidgetPtr->SetHealth(FMath::RoundToInt(Attributes->GetHealth()), FMath::RoundToInt(Attributes->GetMaxHealth()));
		HealthWidget->RequestRenderUpdate();
	}
}

void ACoopArenaEnemyCharacter::OnDeath(AActor* Killer) {
	// handle 2 balls at the same frame when dead
	Attributes->OnDeath.RemoveAll(this);

	if (const auto KillerPlayerStatePtr = Cast<APlayerState>(Killer)) {
		KillerPlayerStatePtr->SetScore(KillerPlayerStatePtr->GetScore() + 1.0f);
		UKismetSystemLibrary::PrintString(
			this, FString::Printf(TEXT("%s: %.0f kills"), *KillerPlayerStatePtr->GetPlayerName(), KillerPlayerStatePtr->GetScore()),
			true, true, FLinearColor::Green, 5.0f
		);
	}

	// only a death inside the standing crowd leaves a gap to re-pack into
	if (bArrivedToStandingPlayer)
		if (const auto GameStatePtr = GetWorld()->GetGameState<ACoopArenaGameState>())
			GameStatePtr->NotifyEnemyDeath();

	Destroy();
}

#if WITH_EDITOR
void ACoopArenaEnemyCharacter::RollStatsFromSpawner() {
	if (const TActorIterator<ACoopArenaEnemySpawner> It(GetWorld()); It)
		Stats = It->GetEnemyNewRolledStats();
}
#endif
