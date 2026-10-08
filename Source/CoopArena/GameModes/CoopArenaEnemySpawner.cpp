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

FCoopArenaEnemyStats ACoopArenaEnemySpawner::GetEnemyNewRolledStats() const {
	FCoopArenaEnemyStats Stats;
	Stats.MaxHealth = FMath::RandRange(MaxHealthRange.Min, MaxHealthRange.Max);

	const float RoleRoll = FMath::FRand();
	if (RoleRoll < CounterRunShare) {
		// counter run
		Stats.ChaseRole = ECoopArenaEnemyChaseRole::CounterRun;
		Stats.OvertakeOrbitRadius = FMath::RandRange(CounterRunOrbitRadiusRange.Min, CounterRunOrbitRadiusRange.Max);
		Stats.OvertakeSpeedFactor = FMath::RandRange(CounterRunOvertakeSpeedFactorRange.Min, CounterRunOvertakeSpeedFactorRange.Max);
	}
	else if (RoleRoll < CounterRunShare + InterceptShare) {
		// intercept
		Stats.ChaseRole = ECoopArenaEnemyChaseRole::Intercept;
		Stats.OvertakeOrbitRadius = FMath::RandRange(InterceptOrbitRadiusRange.Min, InterceptOrbitRadiusRange.Max);
		Stats.OvertakeSpeedFactor = FMath::RandRange(InterceptOvertakeSpeedFactorRange.Min, InterceptOvertakeSpeedFactorRange.Max);
	}
	else {
		// tail
		Stats.ChaseRole = ECoopArenaEnemyChaseRole::Tail;
		Stats.OvertakeOrbitRadius = 0.0f;
		Stats.OvertakeSpeedFactor = 1.0f;
	}

	Stats.SpeedFactor = FMath::RandRange(SpeedFactorRange.Min, SpeedFactorRange.Max);
	return Stats;
}

void ACoopArenaEnemySpawner::SpawnEnemy() {
	if (!EnemyClass)
		return;

	const auto WorldPtr = GetWorld();
	const auto NavSystemPtr = UNavigationSystemV1::GetCurrent(WorldPtr);
	if (!NavSystemPtr)
		return;

	FNavLocation Point;
	if (!NavSystemPtr->GetRandomPoint(Point))
		return;

	// deferred: the stats must be on the enemy before its BeginPlay
	const FTransform Transform(Point.Location + FVector(0.0, 0.0, FMath::RandRange(FallHeightRange.Min, FallHeightRange.Max)));
	const auto EnemyPtr = WorldPtr->SpawnActorDeferred<ACoopArenaEnemyCharacter>(EnemyClass, Transform);
	if (!EnemyPtr)
		return;

	EnemyPtr->SetStats(GetEnemyNewRolledStats());
	EnemyPtr->FinishSpawning(Transform);
	EnemyPtr->OnDestroyed.AddDynamic(this, &ACoopArenaEnemySpawner::OnEnemyDestroyed);
}

void ACoopArenaEnemySpawner::OnEnemyDestroyed(AActor*) {
	if (const auto WorldPtr = GetWorld(); WorldPtr && !WorldPtr->bIsTearingDown)
		SpawnEnemy();
}

static FAutoConsoleCommandWithWorld GCoopArenaSpawnEnemy(
	TEXT("CoopArena.SpawnEnemy"), TEXT("Spawns enemy."),
	FConsoleCommandWithWorldDelegate::CreateLambda([] (const UWorld* World) {
		if (const TActorIterator<ACoopArenaEnemySpawner> It(World); It)
			It->SpawnEnemy();
	})
);
