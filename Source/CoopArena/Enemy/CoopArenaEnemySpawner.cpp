#include "CoopArenaEnemySpawner.h"
#include "CoopArenaEnemyCharacter.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

ACoopArenaEnemySpawner::ACoopArenaEnemySpawner() {
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ACoopArenaEnemySpawner::BeginPlay() {
	Super::BeginPlay();
	SpawnEnemy();
}

void ACoopArenaEnemySpawner::SpawnEnemy() {
	if (!EnemyClass)
		return;

	const auto NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem)
		return;

	FNavLocation Point;
	if (!NavSystem->GetRandomPoint(Point))
		return;

	const auto Height = FMath::FRandRange(FallHeightRange.Min, FallHeightRange.Max);
	const auto Enemy = GetWorld()->SpawnActor<ACoopArenaEnemyCharacter>(
		EnemyClass, Point.Location + FVector(0.0f, 0.0f, Height), FRotator::ZeroRotator
	);

	if (Enemy)
		Enemy->OnDestroyed.AddDynamic(this, &ACoopArenaEnemySpawner::OnEnemyDestroyed);
}

void ACoopArenaEnemySpawner::OnEnemyDestroyed(AActor*) {
	if (const auto World = GetWorld(); World && !World->bIsTearingDown)
		SpawnEnemy();
}

static FAutoConsoleCommandWithWorld GCoopArenaSpawnEnemy(
	TEXT("CoopArena.SpawnEnemy"), TEXT("Spawns enemy."),
	FConsoleCommandWithWorldDelegate::CreateLambda([] (const UWorld* World) {
		if (const auto Spawner = Cast<ACoopArenaEnemySpawner>(UGameplayStatics::GetActorOfClass(World, ACoopArenaEnemySpawner::StaticClass())))
			Spawner->SpawnEnemy();
	})
);
