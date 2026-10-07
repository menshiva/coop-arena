#include "CoopArenaBenchComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "CoopArenaGameplayTags.h"
#include "EngineUtils.h"
#include "EnhancedInputSubsystems.h"
#include "Character/CoopArenaEnemyCharacter.h"
#include "Character/CoopArenaPlayerCharacter.h"
#include "GameFramework/PlayerState.h"

static constexpr double WarmupSeconds = 10.0; // shader compiles and streaming settle, not recorded
static constexpr double MeasureSeconds = 60.0;
static constexpr double RingRadius = 1100.0; // clear of the center platform and inner ramps, short of the outer blocks
static constexpr double RingSteerDistance = 200.0; // this far off the ring the bot heads straight back
static constexpr double TurnRate = 360.0; // degrees per second, a player's pace
static constexpr uint64 AttackPeriodFrames = 2; // a frame down, a frame up: Started needs the action back at None in between
static constexpr double ManeuverPeriodSeconds = 2.5; // a jump once a period, a dash after each
static constexpr double AirDashDelaySeconds = 0.25; // after the jump: near the apex, the jump is 0.68 s in the air
static constexpr double GroundDashDelaySeconds = 1.25; // after the jump: landed
static constexpr double SlowFrameMs = 1000.0 / 30.0;

UCoopArenaBenchComponent::UCoopArenaBenchComponent() {
	PrimaryComponentTick.bCanEverTick = true;
}

void UCoopArenaBenchComponent::BeginPlay() {
	Super::BeginPlay();

	// after the controller
	AddTickPrerequisiteActor(GetOwner());

	// GE_Damage skips targets with this tag
	if (const auto AbilitySystemPtr = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner<APlayerController>()->PlayerState))
		AbilitySystemPtr->AddLooseGameplayTag(CoopArena_Status_Invulnerable);

	StartTime = FPlatformTime::Seconds();
	LastFrameTime = StartTime;
}

void UCoopArenaBenchComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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

	if (Elapsed >= WarmupSeconds) {
		if (FrameTimesMs.IsEmpty()) {
			// -csvStartOnEvent=BenchStart starts the capture here
			CSV_EVENT_GLOBAL(TEXT("BenchStart"));
		}
		FrameTimesMs.Push((Now - LastFrameTime) * 1000.0);
	}
	LastFrameTime = Now;

	Drive(PrevElapsed, Elapsed);
}

void UCoopArenaBenchComponent::Drive(const double PrevElapsed, const double Elapsed) {
	const auto ControllerPtr = GetOwner<APlayerController>();
	const auto CharacterPtr = ControllerPtr->GetPawn<ACoopArenaPlayerCharacter>();
	const auto InputPtr = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(ControllerPtr->GetLocalPlayer());
	if (!CharacterPtr || !InputPtr)
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
		// buttons: balls nonstop; a jump once a period, a dash after it, in the air and on the ground in turn
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

void UCoopArenaBenchComponent::WriteResults() const {
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

	const auto Path = FPaths::ProfilingDir() / TEXT("Bench.csv");
	FString Text;
	if (!IFileManager::Get().FileExists(*Path))
		Text = TEXT("Time,Config,ShaderPlatform,Resolution,Frames,AvgMs,P50Ms,P95Ms,P99Ms,MaxMs,SlowFrames,CommandLine\n");
	Text += FString::Printf(
		TEXT("%s,%s,%s,%dx%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%d,\"%s\"\n"),
		*FDateTime::Now().ToString(), LexToString(FApp::GetBuildConfiguration()), *FDataDrivenShaderPlatformInfo::GetName(GMaxRHIShaderPlatform).ToString(),
		GSystemResolution.ResX, GSystemResolution.ResY, Sorted.Num(),
		TotalMs / Sorted.Num(), Percentile(0.5), Percentile(0.95), Percentile(0.99), Sorted.Last(), SlowFrames,
		*FString(FCommandLine::Get()).Replace(TEXT("\""), TEXT("\"\""))
	);
	FFileHelper::SaveStringToFile(Text, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM, &IFileManager::Get(), FILEWRITE_Append);
}
