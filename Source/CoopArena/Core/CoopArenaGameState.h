#pragma once

#include "GameFramework/GameStateBase.h"
#include "CoopArenaGameState.generated.h"

UCLASS()
class ACoopArenaGameState : public AGameStateBase {
	GENERATED_BODY()
public:
	FORCEINLINE static ACoopArenaGameState* Get(const UWorld* World) {
		return World->GetGameState<ACoopArenaGameState>();
	}

	static uint32 GetEnemyDeathCount(const UWorld* World) {
		if (const auto GameState = Get(World))
			return GameState->GetEnemyDeathCount();
		return 0;
	}

	FORCEINLINE uint32 GetEnemyDeathCount() const { return EnemyDeathCount; }
	FORCEINLINE void NotifyEnemyDeath() { ++EnemyDeathCount; }
private:
	uint32 EnemyDeathCount = 0;
};
