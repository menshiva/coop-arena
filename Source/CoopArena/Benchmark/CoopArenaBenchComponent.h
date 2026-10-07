#pragma once

#include "Components/ActorComponent.h"
#include "CoopArenaBenchComponent.generated.h"

// benchmark component
// a bot runs the ring around the arena center throwing balls nonstop, jumping and dashing now and then
// appends frame time stats to Saved/Profiling/Bench.csv and quits
UCLASS()
class UCoopArenaBenchComponent : public UActorComponent {
	GENERATED_BODY()
public:
	UCoopArenaBenchComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	void Drive(double PrevElapsed, double Elapsed);
	void WriteResults() const;

	double StartTime = 0.0;
	double LastFrameTime = 0.0;
	bool bFinished = false;

	TArray<double> FrameTimesMs;
	TWeakObjectPtr<AActor> Target;
};
