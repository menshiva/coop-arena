#include "CoopArenaBenchSubsystem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "CoopArenaGameplayTags.h"
#include "EngineUtils.h"
#include "EnhancedInputSubsystems.h"
#include "Character/CoopArenaPlayerCharacter.h"
#include "GameFramework/PlayerState.h"
#include "GameModes/CoopArenaEnemySpawner.h"
#include "Performance/EnginePerformanceTargets.h"
#include "Projectiles/CoopArenaProjectileManager.h"
THIRD_PARTY_INCLUDES_START
#include <dxgi1_4.h>
THIRD_PARTY_INCLUDES_END

static constexpr double WarmupSeconds = 10.0; // shader compiles and streaming settle, out of the frame stats
static constexpr double MeasureSeconds = 60.0;
static constexpr double RingRadius = 1100.0; // clear of the center platform and inner ramps, short of the outer blocks
static constexpr double RingSteerDistance = 200.0; // this far off the ring the bot heads straight back
static constexpr double BallsScatterRadius = 1600.0; // -BenchBalls: around the arena center, short of the outer columns at 16.5 m
static constexpr double TurnRate = 360.0; // degrees per second, a player's pace
static constexpr uint64 AttackPeriodFrames = 2; // a frame down, a frame up: Started needs the action back at None in between
static constexpr double ManeuverPeriodSeconds = 2.5; // a jump once a period, a dash after each
static constexpr double AirDashDelaySeconds = 0.25; // after the jump: near the apex, the jump is 0.68 s in the air
static constexpr double GroundDashDelaySeconds = 1.25; // after the jump: landed
static constexpr double SlowFrameMs = 1000.0 / 30.0;
static constexpr double BytesPerMiB = 1024.0 * 1024.0;

// the process's video memory, as the Task Manager shows it (RHIGetMemoryStats isn't refreshed in Shipping)
static uint64 GetVideoMemoryUsage() {
	TRefCountPtr<IDXGIFactory1> Factory;
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(Factory.GetInitReference()))))
		return 0;

	// the adapter the RHI renders on
	TRefCountPtr<IDXGIAdapter1> Adapter;
	for (uint32 i = 0; Factory->EnumAdapters1(i, Adapter.GetInitReference()) == S_OK; ++i) {
		DXGI_ADAPTER_DESC1 Desc;
		TRefCountPtr<IDXGIAdapter3> Adapter3;
		DXGI_QUERY_VIDEO_MEMORY_INFO Info;
		if (SUCCEEDED(Adapter->GetDesc1(&Desc)) && Desc.VendorId == GRHIVendorId && Desc.DeviceId == GRHIDeviceId
			&& SUCCEEDED(Adapter->QueryInterface(IID_PPV_ARGS(Adapter3.GetInitReference())))
			&& SUCCEEDED(Adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &Info)))
			return Info.CurrentUsage;
	}

	return 0;
}

bool UCoopArenaBenchSubsystem::ShouldCreateSubsystem(UObject* Outer) const {
	return FParse::Param(FCommandLine::Get(), TEXT("bench")) && Super::ShouldCreateSubsystem(Outer);
}

void UCoopArenaBenchSubsystem::OnWorldBeginPlay(UWorld& InWorld) {
	Super::OnWorldBeginPlay(InWorld);

	{
		// set enemy number to spawn
		int32 EnemyNum = 0;
		const TActorIterator<ACoopArenaEnemySpawner> SpawnerIt(&InWorld);
		if (SpawnerIt && FParse::Value(FCommandLine::Get(), TEXT("BenchEnemies="), EnemyNum))
			SpawnerIt->SetEnemyNum(EnemyNum);
	}

	{
		// make player invulnerable to the enemies
		const auto ControllerPtr = InWorld.GetFirstPlayerController();
		if (const auto AbilitySystemPtr = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControllerPtr ? ControllerPtr->PlayerState.Get() : nullptr))
			AbilitySystemPtr->AddLooseGameplayTag(CoopArena_Status_Invulnerable);
	}
}

void UCoopArenaBenchSubsystem::Tick(const float DeltaTime) {
	Super::Tick(DeltaTime);

	if (StartTime == 0.0) {
		// the first frame of play: loading (since the process start) is over, the balls go down, the clock starts
		ScatterBalls();
		StartTime = FPlatformTime::Seconds();
		LastFrameTime = StartTime;
	}

	const double Now = FPlatformTime::Seconds();
	const double Elapsed = Now - StartTime;
	const double PrevElapsed = LastFrameTime - StartTime;

	if (Elapsed >= WarmupSeconds + MeasureSeconds) {
		if (!bFinished) {
			// -csvStopOnEvent=BenchEnd stops the capture here
			CSV_EVENT_GLOBAL(TEXT("BenchEnd"));
			WriteResults();
			bFinished = true;
		}

#if CSV_PROFILER
		if (FCsvProfiler::Get()->IsCapturing() || FCsvProfiler::Get()->IsWritingFile())
			return; // let the capture write its file
#endif

		FPlatformMisc::RequestExit(false, TEXT("CoopArenaBench"));
		return;
	}

	const double FrameMs = (Now - LastFrameTime) * 1000.0;
	if (Elapsed >= WarmupSeconds) {
		if (FrameTimesMs.IsEmpty()) {
			// -csvStartOnEvent=BenchStart starts the capture here
			CSV_EVENT_GLOBAL(TEXT("BenchStart"));
		}
		FrameTimesMs.Push(FrameMs);
		// the engine's own thread and GPU times, as stat unit shows them: a frame or two behind, fine for the averages
		GameThreadTotalMs += FPlatformTime::ToMilliseconds(GGameThreadTime);
		RenderThreadTotalMs += FPlatformTime::ToMilliseconds(GRenderThreadTime);
		GPUTotalMs += FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());
	}
	else {
		// warmup: only its worst frame and hitches go to the file
		WarmupMaxMs = FMath::Max(WarmupMaxMs, FrameMs);
		if (FrameMs >= FEnginePerformanceTargets::GetHitchFrameTimeThresholdMS())
			++WarmupHitches;
	}
	LastFrameTime = Now;

	Drive(PrevElapsed, Elapsed);
}

void UCoopArenaBenchSubsystem::ScatterBalls() const {
	// spawn balls scattered over the arena floor

	const TActorIterator<ACoopArenaProjectileManager> ManagerIt(GetWorld());
	if (!ManagerIt)
		return;

	int32 BallNum = 0;
	FParse::Value(FCommandLine::Get(), TEXT("BenchBalls="), BallNum);
	if (BallNum <= 0)
		return;

	const double Lift = ManagerIt->GetProjectileRadius();
	if (Lift <= 0.0)
		return;

	const FRandomStream Random(0);
	TArray<FTransform> Transforms;
	Transforms.Reserve(BallNum);
	for (int32 i = 0; i < BallNum; ++i) {
		// uniform over the disk, then straight down onto the floor, a ramp or the center platform
		const auto Point = FVector2D(BallsScatterRadius * FMath::Sqrt(Random.FRand()), 0.0).GetRotated(Random.FRandRange(0.0, 360.0));
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByObjectType(Hit, FVector(Point, 1000.0), FVector(Point, -1000.0), ECC_WorldStatic))
			Transforms.Push(FTransform(Hit.Location + FVector(0.0, 0.0, Lift)));
	}
	ManagerIt->AddRestingBalls(Transforms);
}

void UCoopArenaBenchSubsystem::Drive(const double PrevElapsed, const double Elapsed) {
	const auto ControllerPtr = GetWorld()->GetFirstPlayerController();
	const auto CharacterPtr = ControllerPtr ? ControllerPtr->GetPawn<ACoopArenaPlayerCharacter>() : nullptr;
	const auto InputPtr = CharacterPtr ? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(ControllerPtr->GetLocalPlayer()) : nullptr;
	if (!InputPtr)
		return;

	const auto Location = CharacterPtr->GetActorLocation();
	const auto ControlRotation = ControllerPtr->GetControlRotation();

	{
		// move: counter-clockwise along the ring around the world origin (the arena center), steering back onto it
		const FVector2D Offset(Location);
		const double Distance = Offset.Size();
		const auto Outward = Distance > UE_KINDA_SMALL_NUMBER ? Offset / Distance : FVector2D(1.0, 0.0);
		const FVector2D Tangent(-Outward.Y, Outward.X);
		const auto Direction = Tangent + Outward * FMath::Clamp((RingRadius - Distance) / RingSteerDistance, -1.0, 1.0);

		// move takes X right, Y forward in the camera's yaw
		const auto CameraSpace = FRotator(0.0, ControlRotation.Yaw, 0.0).UnrotateVector(FVector(Direction.GetSafeNormal(), 0.0));
		InputPtr->InjectInputForAction(CharacterPtr->MoveAction, FInputActionValue(FVector2D(CameraSpace.Y, CameraSpace.X)), {}, {});
	}

	{
		// look: hold the target until it dies, then take the nearest enemy; the arena center when there are none
		if (!Target.IsValid()) {
			double TargetDistSquared = TNumericLimits<double>::Max();
			for (TActorIterator<ACoopArenaEnemyCharacter> It(GetWorld()); It; ++It) {
				const double DistSquared = FVector::DistSquared(Location, It->GetActorLocation());
				if (DistSquared < TargetDistSquared) {
					TargetDistSquared = DistSquared;
					Target = *It;
				}
			}
		}
		const auto AimPoint = Target.IsValid() ? Target->GetActorLocation() : FVector::ZeroVector;

		// turn at a player's pace; look takes degrees per frame (bEnableLegacyInputScales=False)
		const auto Delta = ((AimPoint - ControllerPtr->PlayerCameraManager->GetCameraLocation()).Rotation() - ControlRotation).GetNormalized();
		const double MaxStep = TurnRate * (Elapsed - PrevElapsed);
		const FVector2D Step(FMath::Clamp(Delta.Yaw, -MaxStep, MaxStep), FMath::Clamp(Delta.Pitch, -MaxStep, MaxStep));
		InputPtr->InjectInputForAction(CharacterPtr->LookAction, FInputActionValue(Step), {}, {});
	}

	{
		// actions: balls nonstop; a jump once a period, a dash after it, in the air and on the ground in turn
		const auto Beat = [PrevElapsed, Elapsed] (const double Period, const double Offset) {
			// a beat every Period seconds, shifted by Offset, falls on this frame
			return FMath::FloorToInt64((Elapsed - Offset) / Period) != FMath::FloorToInt64((PrevElapsed - Offset) / Period);
		};
		const bool bAttack = GFrameCounter % AttackPeriodFrames == 0;
		const bool bJump = Beat(ManeuverPeriodSeconds, 0.0);
		const bool bDash = Beat(2.0 * ManeuverPeriodSeconds, AirDashDelaySeconds) || Beat(2.0 * ManeuverPeriodSeconds, ManeuverPeriodSeconds + GroundDashDelaySeconds);

		if (bJump)
			InputPtr->InjectInputForAction(CharacterPtr->JumpAction, FInputActionValue(true), {}, {});
		for (const auto& Binding : CharacterPtr->AbilityBindings)
			if (Binding.InputAction && (Binding.AbilityTag == CoopArena_Ability_Dash ? bDash : bAttack))
				InputPtr->InjectInputForAction(Binding.InputAction, FInputActionValue(true), {}, {});
	}
}

void UCoopArenaBenchSubsystem::WriteResults() const {
	auto Sorted = FrameTimesMs;
	Sorted.Sort();
	const auto Percentile = [&Sorted] (const double Fraction) {
		return Sorted[FMath::Clamp(FMath::CeilToInt32(Fraction * Sorted.Num()) - 1, 0, Sorted.Num() - 1)];
	};

	double TotalMs = 0.0;
	int32 SlowFrames = 0;
	for (const double Ms : Sorted) {
		TotalMs += Ms;
		if (Ms > SlowFrameMs)
			++SlowFrames;
	}

	// memory at the end of the run: RAM (working set), video memory of the process and of all textures, render targets included
	const auto Memory = FPlatformMemory::GetStats();
	FTextureMemoryStats TextureMemory;
	RHIGetTextureMemoryStats(TextureMemory);

	const auto Path = FPaths::ProfilingDir() / TEXT("Bench.csv");
	FString Text;
	if (!IFileManager::Get().FileExists(*Path))
		Text = TEXT("Time,Config,ShaderPlatform,Resolution,Frames,AvgMs,P50Ms,P95Ms,P99Ms,MaxMs,SlowFrames,GameThreadMs,RenderThreadMs,GPUMs,LoadSec,WarmupMaxMs,WarmupHitches,RamMiB,RamPeakMiB,VramMiB,TexturesMiB,CommandLine\n");
	Text += FString::Printf(
		TEXT("%s,%s,%s,%dx%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%.1f,%.1f,%.1f,%.1f,\"%s\"\n"),
		*FDateTime::Now().ToString(), LexToString(FApp::GetBuildConfiguration()), *FDataDrivenShaderPlatformInfo::GetName(GMaxRHIShaderPlatform).ToString(),
		GSystemResolution.ResX, GSystemResolution.ResY, Sorted.Num(),
		TotalMs / Sorted.Num(), Percentile(0.5), Percentile(0.95), Percentile(0.99), Sorted.Last(), SlowFrames,
		GameThreadTotalMs / Sorted.Num(), RenderThreadTotalMs / Sorted.Num(), GPUTotalMs / Sorted.Num(),
		StartTime - GStartTime, WarmupMaxMs, WarmupHitches,
		Memory.UsedPhysical / BytesPerMiB, Memory.PeakUsedPhysical / BytesPerMiB, GetVideoMemoryUsage() / BytesPerMiB, (TextureMemory.StreamingMemorySize + TextureMemory.NonStreamingMemorySize) / BytesPerMiB,
		*FString(FCommandLine::Get()).Replace(TEXT("\""), TEXT("\"\""))
	);
	FFileHelper::SaveStringToFile(Text, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM, &IFileManager::Get(), FILEWRITE_Append);
}
