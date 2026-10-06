#include "CoopArenaBTService_Approach.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/CoopArenaEnemyCharacter.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameModes/CoopArenaGameState.h"

#if ENABLE_DRAW_DEBUG
static TAutoConsoleVariable CVarDebugApproach(TEXT("CoopArena.DebugApproach"), false, TEXT("Draws where each enemy runs"));
#endif

UCoopArenaBTService_Approach::UCoopArenaBTService_Approach() {
	NodeName = "Approach";
	Interval = 0.25f;

	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UCoopArenaBTService_Approach, TargetActorKey), AActor::StaticClass());
	ApproachLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UCoopArenaBTService_Approach, ApproachLocationKey));
	ArrivedKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UCoopArenaBTService_Approach, ArrivedKey));
}

void UCoopArenaBTService_Approach::InitializeFromAsset(UBehaviorTree& Asset) {
	Super::InitializeFromAsset(Asset);

	if (const auto BlackboardPtr = GetBlackboardAsset()) {
		TargetActorKey.ResolveSelectedKey(*BlackboardPtr);
		ApproachLocationKey.ResolveSelectedKey(*BlackboardPtr);
		ArrivedKey.ResolveSelectedKey(*BlackboardPtr);
	}
}

void UCoopArenaBTService_Approach::InitializeMemory(UBehaviorTreeComponent&, uint8* NodeMemory, const EBTMemoryInit::Type InitType) const {
	InitializeNodeMemory<FMemory>(NodeMemory, InitType);
}

void UCoopArenaBTService_Approach::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, const float DeltaSeconds) {
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const auto BlackboardPtr = OwnerComp.GetBlackboardComponent();
	if (!BlackboardPtr)
		return;
	const auto TargetActorKeyID = TargetActorKey.GetSelectedKeyID();
	const auto LocationKeyID = ApproachLocationKey.GetSelectedKeyID();
	const auto ArrivedKeyID = ArrivedKey.GetSelectedKeyID();
	auto& Memory = *CastInstanceNodeMemory<FMemory>(NodeMemory);

	const auto ControllerPtr = OwnerComp.GetAIOwner();
	const auto EnemyPtr = ControllerPtr ? ControllerPtr->GetPawn<ACoopArenaEnemyCharacter>() : nullptr;
	const auto PlayerPtr = Cast<AActor>(BlackboardPtr->GetValue<UBlackboardKeyType_Object>(TargetActorKeyID));
	if (!EnemyPtr || !PlayerPtr) {
		BlackboardPtr->ClearValue(LocationKeyID);
		BlackboardPtr->ClearValue(ArrivedKeyID);
		if (EnemyPtr) {
			EnemyPtr->SetArrivedToStandingPlayer(false);
			EnemyPtr->SetOvertaking(false);
		}
		Memory.StalledFor = 0.0f;
		return;
	}
	const auto NavSystemPtr = UNavigationSystemV1::GetCurrent(EnemyPtr->GetWorld());

	if (!HandleOffNavmesh(NavSystemPtr, *EnemyPtr, Memory))
		return;

	// approach
	const auto PointOpt = ChasePoint(NavSystemPtr, *EnemyPtr, *PlayerPtr, Memory);
	if (const auto PointPtr = PointOpt.GetPtrOrNull())
		BlackboardPtr->SetValue<UBlackboardKeyType_Vector>(LocationKeyID, *PointPtr);
	else
		BlackboardPtr->ClearValue(LocationKeyID);

	// arrived
	const bool bArrived = IsArrived(*EnemyPtr, *PlayerPtr, Memory, DeltaSeconds);
	BlackboardPtr->SetValue<UBlackboardKeyType_Bool>(ArrivedKeyID, bArrived);

#if ENABLE_DRAW_DEBUG
	if (CVarDebugApproach.GetValueOnGameThread()) {
		const auto WorldPtr = EnemyPtr->GetWorld();
		const auto EnemyPos = EnemyPtr->GetActorLocation();
		const auto PlayerPos = PlayerPtr->GetActorLocation();
		const float Life = Interval + RandomDeviation;
		const FColor Color = !PointOpt.IsSet() ? FColor::White : EnemyPtr->GetChaseRole() == ECoopArenaEnemyChaseRole::CounterRun ? FColor::Cyan : FColor::Yellow;
		DrawDebugLine(WorldPtr, EnemyPos, PointOpt.Get(PlayerPos), Color, false, Life, 0, 2.0f);
		if (PointOpt.IsSet())
			DrawDebugSphere(WorldPtr, PointOpt.GetValue(), 30.0f, 8, Color, false, Life);
		if (bArrived)
			DrawDebugSphere(WorldPtr, EnemyPos + FVector(0.0, 0.0, 120.0), 15.0f, 8, FColor::Red, false, Life);
	}
#endif
}

bool UCoopArenaBTService_Approach::HandleOffNavmesh(const UNavigationSystemV1* NavSystem, ACoopArenaEnemyCharacter& Enemy, FMemory& Memory) {
	const auto EnemyPos = Enemy.GetActorLocation();
	FNavLocation OnNavmesh;
	if (NavSystem && Enemy.GetCharacterMovement()->IsMovingOnGround() && !NavSystem->ProjectPointToNavigation(EnemyPos, OnNavmesh)) {
		if (++Memory.OffNavmeshTicks < 4)
			return false;
		Memory.OffNavmeshTicks = 0;
		if (NavSystem->ProjectPointToNavigation(EnemyPos, OnNavmesh, FVector(500.0))) {
			// nudge back to the closest point
			Enemy.SetActorLocation(
				OnNavmesh.Location + FVector(0.0, 0.0, Enemy.GetSimpleCollisionHalfHeight()), false,
				nullptr, ETeleportType::TeleportPhysics
			);
		}
		else {
			Enemy.Destroy();
		}
		return false;
	}
	Memory.OffNavmeshTicks = 0;
	return true;
}

TOptional<FVector> UCoopArenaBTService_Approach::ChasePoint(const UNavigationSystemV1* NavSystem, ACoopArenaEnemyCharacter& Enemy, const AActor& Player, FMemory& Memory) const {
	auto Velocity = FVector2D(Player.GetVelocity());
	const auto Role = Enemy.GetChaseRole();
	if (!NavSystem || Role == ECoopArenaEnemyChaseRole::Tail || Velocity.IsNearlyZero()) {
		// a tail and everyone around a standing player go straight at him
		Enemy.SetOvertaking(false);
		return {};
	}

	// a dash sets the velocity by root motion above MaxWalkSpeed - clamp it
	if (const auto CharacterPtr = Cast<ACharacter>(&Player))
		Velocity = Velocity.GetClampedToMaxSize(CharacterPtr->GetCharacterMovement()->MaxWalkSpeed);

	// the enemy in the player's frame of motion: bearing 0 - ahead of him, 180 - behind, and the side he is on
	const auto PlayerPos = Player.GetActorLocation();
	const auto EnemyPos = Enemy.GetActorLocation();
	const auto Forward = Velocity.GetSafeNormal();
	const auto Relative = FVector2D(EnemyPos - PlayerPos);
	const double Bearing = FMath::RadiansToDegrees(FMath::Acos(FVector2D::DotProduct(Forward, Relative.GetSafeNormal())));
	const double Lateral = FVector2D::CrossProduct(Forward, Relative);

	// update the side if not right behind the player
	static constexpr double SideDeadband = 50.0;
	if (FMath::Abs(Lateral) > SideDeadband)
		Memory.bRightOrbitSide = Lateral > 0.0;

	// caught behind - go around the player on the role's arc up to the role's bearing, then attack from there (the actor branch)
	const double ArcEndBearing = Role == ECoopArenaEnemyChaseRole::CounterRun ? CounterRunArcEndBearing : InterceptArcEndBearing;
	if (!Enemy.IsOvertaking() && Bearing > ArcStartBearing)
		Enemy.SetOvertaking(true);
	else if (Enemy.IsOvertaking() && Bearing <= ArcEndBearing)
		Enemy.SetOvertaking(false);
	if (!Enemy.IsOvertaking())
		return {};

	const double OrbitSign = Memory.bRightOrbitSide ? 1.0 : -1.0;
	const double OrbitRadius = Enemy.GetOvertakeOrbitRadius();
	const double NextBearing = Bearing - (OrbitRadius > 0.0 ? FMath::RadiansToDegrees(ArcStepDistance / OrbitRadius) : 0.0);

	// offset pursuit: https://www.red3d.com/cwr/steer/gdc99/#:~:text=Figure%205%3A%20offset%20pursuit
	// the target is a point held at an offset in the moving target's frame
	// the point moves with the player - aim where it will be when the enemy gets there (pursuit lead, T = distance / speed)
	auto Point = FVector2D(PlayerPos) + Forward.GetRotated(OrbitSign * NextBearing) * OrbitRadius;
	if (const double EnemySpeed = Enemy.GetCharacterMovement()->MaxWalkSpeed; EnemySpeed > 0.0) {
		// https://sourceforge.net/p/opensteer/code/HEAD/tree/trunk/include/OpenSteer/SteerLibrary.h#l936
		const double T = FVector2D::Distance(Point, FVector2D(EnemyPos)) / EnemySpeed;

		static constexpr double LeadTimeMax = 1.0; // how far ahead the moving arc point is aimed at
		Point += Velocity * FMath::Min(T, LeadTimeMax);
	}

	FNavLocation Projected;
	if (NavSystem->ProjectPointToNavigation(FVector(Point.X, Point.Y, PlayerPos.Z), Projected, ProjectionExtent))
		return Projected.Location;

	return {};
}

bool UCoopArenaBTService_Approach::IsArrived(ACoopArenaEnemyCharacter& Enemy, const AActor& Player, FMemory& Memory, const float DeltaSeconds) const {
	if (!Player.GetVelocity().IsNearlyZero() || !Enemy.GetCharacterMovement()->IsMovingOnGround()) {
		Enemy.SetArrivedToStandingPlayer(false);
		Memory.StalledFor = 0.0f;
		return false;
	}
	if (Enemy.IsArrivedToStandingPlayer())
		return true;

	const auto Arrive = [&] {
		Enemy.SetArrivedToStandingPlayer(true);
		Memory.StalledFor = 0.0f;
		return true;
	};

	if (Enemy.GetVelocity().SizeSquared2D() < FMath::Square(SpeedConsideredAsStall))
		Memory.StalledFor += DeltaSeconds;
	else
		Memory.StalledFor = 0.0f;

	if (Memory.StalledFor >= TimeConsideredAsStall)
		return Arrive();

	// an enemy death dropped the latch - give the crowd a moment to move into the gap before the neighbors anchor it again
	bool bNeighboursAnchor = true;
	if (Enemy.IsRepackingAfterDeath()) {
		const uint32 DeathCount = ACoopArenaGameState::GetEnemyDeathCount(Enemy.GetWorld());
		if (Memory.RepackingDeathCount != DeathCount) {
			// every further death restarts the window
			Memory.RepackingDeathCount = DeathCount;
			Memory.RepackingFor = 0.0f;
		}
		Memory.RepackingFor += DeltaSeconds;
		bNeighboursAnchor = Memory.RepackingFor >= TimeToRepackAfterDeath;
	}

	TArray<FOverlapResult> Overlaps;
	Enemy.GetWorld()->OverlapMultiByObjectType(
		Overlaps, Enemy.GetActorLocation(), FQuat::Identity, FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Enemy.GetSimpleCollisionRadius() + TouchPackingGapDistance),
		FCollisionQueryParams(NAME_None, false, &Enemy)
	);
	for (const auto& Overlap : Overlaps) {
		if (Overlap.GetActor() == &Player)
			return Arrive();

		if (bNeighboursAnchor)
			if (const auto OtherPtr = Cast<ACoopArenaEnemyCharacter>(Overlap.GetActor()))
				if (OtherPtr->IsArrivedToStandingPlayer())
					return Arrive();
	}

	return false;
}
