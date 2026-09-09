#include "CoopArenaProjectileMovement.h"
#include "CoopArenaProjectileManager.h"

void UCoopArenaProjectileMovement::BindToSlot(ACoopArenaProjectileManager* InManager, const int32 InSlotIndex) {
	Manager = InManager;
	SlotIndex = InSlotIndex;
}

void UCoopArenaProjectileMovement::HandleImpact(const FHitResult& Hit, const float TimeSlice, const FVector& MoveDelta) {
	if (Manager.IsValid())
		Manager->OnBallImpact(SlotIndex, Hit);

	Super::HandleImpact(Hit, TimeSlice, MoveDelta);
}

void UCoopArenaProjectileMovement::StopSimulating(const FHitResult& HitResult) {
	Super::StopSimulating(HitResult);

	if (Manager.IsValid())
		Manager->OnBallStopped(SlotIndex);
}
