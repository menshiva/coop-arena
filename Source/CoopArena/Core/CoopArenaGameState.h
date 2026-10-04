#pragma once

#include "GameFramework/GameStateBase.h"
#include "CoopArenaGameState.generated.h"

UCLASS()
class ACoopArenaGameState : public AGameStateBase {
	GENERATED_BODY()
public:
	static uint32 GetEnemyDeathCount(const UWorld* World) {
		if (const auto GameStatePtr = World->GetGameState<ACoopArenaGameState>())
			return GameStatePtr->GetEnemyDeathCount();
		return 0;
	}

	FORCEINLINE uint32 GetEnemyDeathCount() const { return EnemyDeathCount; }
	FORCEINLINE void NotifyEnemyDeath() { ++EnemyDeathCount; }
private:
	uint32 EnemyDeathCount = 0;
};
