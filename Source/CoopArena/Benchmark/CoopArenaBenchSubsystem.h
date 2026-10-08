#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "CoopArenaBenchSubsystem.generated.h"

// benchmark, exists only with -bench
// a bot runs the ring around the arena center throwing balls nonstop, jumping and dashing now and then
// -BenchEnemies=N sets the enemy count, with -BenchBalls=N the arena starts with N balls already at rest
// appends frame time, load and memory stats to Saved/Profiling/Bench.csv and quits
UCLASS()
class UCoopArenaBenchSubsystem : public UTickableWorldSubsystem {
	GENERATED_BODY()
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UCoopArenaBenchSubsystem, STATGROUP_Tickables); }
private:
	void ScatterBalls() const;
	void Drive(double PrevElapsed, double Elapsed);
	void WriteResults() const;

	double StartTime = 0.0;
	double LastFrameTime = 0.0;
	bool bFinished = false;

	double WarmupMaxMs = 0.0;
	int32 WarmupHitches = 0;

	TArray<double> FrameTimesMs;
	TWeakObjectPtr<AActor> Target;
};
