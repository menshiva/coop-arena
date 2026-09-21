#include "CoopArenaEnemySpawner.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"

ACoopArenaEnemySpawner::ACoopArenaEnemySpawner() {
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void ACoopArenaEnemySpawner::BeginPlay() {
	Super::BeginPlay();
	for (int32 i = 0; i < EnemyNum; ++i)
		SpawnEnemy();
}

void ACoopArenaEnemySpawner::SpawnEnemy() {
	if (!EnemyClass)
		return;

	const auto World = GetWorld();
	const auto NavSystem = UNavigationSystemV1::GetCurrent(World);
	if (!NavSystem)
		return;

	FNavLocation Point;
	if (!NavSystem->GetRandomPoint(Point))
		return;

	// deferred: the stats must be on the enemy before its BeginPlay
	const FTransform Transform(Point.Location + FVector(0.0f, 0.0f, FMath::FRandRange(FallHeightRange.Min, FallHeightRange.Max)));
	const auto Enemy = World->SpawnActorDeferred<ACoopArenaEnemyCharacter>(EnemyClass, Transform);
	if (!Enemy)
		return;

	Enemy->SetStats(GetEnemyNewRolledStats());
	Enemy->FinishSpawning(Transform);
	Enemy->OnDestroyed.AddDynamic(this, &ACoopArenaEnemySpawner::OnEnemyDestroyed);
}

FCoopArenaEnemyStats ACoopArenaEnemySpawner::GetEnemyNewRolledStats() const {
	FCoopArenaEnemyStats Stats;
	Stats.MaxHealth = FMath::RandRange(MaxHealthRange.Min, MaxHealthRange.Max);

	const float RoleRoll = FMath::FRand();
	if (RoleRoll < CounterRunShare) {
		Stats.ChaseRole = ECoopArenaEnemyChaseRole::CounterRun;
		Stats.OvertakeOrbitRadius = FMath::RandRange(CounterRunOrbitRadiusRange.Min, CounterRunOrbitRadiusRange.Max);
		Stats.OvertakeSpeedFactor = FMath::RandRange(CounterRunOvertakeSpeedFactorRange.Min, CounterRunOvertakeSpeedFactorRange.Max);
	}
	else if (RoleRoll < CounterRunShare + InterceptShare) {
		Stats.ChaseRole = ECoopArenaEnemyChaseRole::Intercept;
		Stats.OvertakeOrbitRadius = FMath::RandRange(InterceptOrbitRadiusRange.Min, InterceptOrbitRadiusRange.Max);
		Stats.OvertakeSpeedFactor = FMath::RandRange(InterceptOvertakeSpeedFactorRange.Min, InterceptOvertakeSpeedFactorRange.Max);
	}
	else
		Stats.ChaseRole = ECoopArenaEnemyChaseRole::Tail;

	Stats.SpeedFactor = FMath::RandRange(SpeedFactorRange.Min, SpeedFactorRange.Max);
	return Stats;
}

void ACoopArenaEnemySpawner::OnEnemyDestroyed(AActor*) {
	if (const auto World = GetWorld(); World && !World->bIsTearingDown)
		SpawnEnemy();
}

static FAutoConsoleCommandWithWorld GCoopArenaSpawnEnemy(
	TEXT("CoopArena.SpawnEnemy"), TEXT("Spawns enemy."),
	FConsoleCommandWithWorldDelegate::CreateLambda([] (const UWorld* World) {
		if (const TActorIterator<ACoopArenaEnemySpawner> It(World); It)
			It->SpawnEnemy();
	})
);
