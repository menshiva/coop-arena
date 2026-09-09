#pragma once

#include "GameFramework/Character.h"
#include "CoopArenaEnemyCharacter.generated.h"

UCLASS(Abstract)
class ACoopArenaEnemyCharacter : public ACharacter {
	GENERATED_BODY()
public:
	virtual void PostActorCreated() override;

#if WITH_EDITOR
	virtual void PostEditImport() override;
#endif
protected:
	UPROPERTY(EditAnywhere, Category="Health")
	FFloatInterval HealthRange = FFloatInterval(50.0f, 150.0f);

	UPROPERTY(EditAnywhere, Category="Health")
	float Health = 0.0f;
private:
	void SetRandomHealth();
};
