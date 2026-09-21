#include "CoopArenaProjectileMovement.h"
#include "CoopArenaProjectileManager.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"

UCoopArenaProjectileMovement::UCoopArenaProjectileMovement() {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
}

void UCoopArenaProjectileMovement::Init(ACoopArenaProjectileManager* InManager, const int32 InSlotIndex) {
	Manager = InManager;
	SlotIndex = InSlotIndex;
}

void UCoopArenaProjectileMovement::HandleImpact(const FHitResult& Hit, const float TimeSlice, const FVector& MoveDelta) {
	if (Manager.IsValid())
		Manager->OnBallImpact(SlotIndex, Hit);

	Super::HandleImpact(Hit, TimeSlice, MoveDelta);
}

bool UCoopArenaProjectileMovement::CheckStillInWorld() {
	// the engine checks KillZ against the owner (manager), not the ball
	if (UpdatedComponent) {
		const auto WorldSettings = GetWorld()->GetWorldSettings();
		if (WorldSettings && WorldSettings->AreWorldBoundsChecksEnabled() && UpdatedComponent->GetComponentLocation().Z < WorldSettings->KillZ) {
			Super::StopSimulating(FHitResult());
			if (Manager.IsValid())
				Manager->OnBallStopped(SlotIndex, false);
			return false;
		}
	}
	return Super::CheckStillInWorld();
}

void UCoopArenaProjectileMovement::StopSimulating(const FHitResult& HitResult) {
	Super::StopSimulating(HitResult);

	if (Manager.IsValid())
		Manager->OnBallStopped(SlotIndex, true);
}
