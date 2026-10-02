#pragma once

#include "Blueprint/UserWidget.h"
#include "CoopArenaEnemyIndicatorWidget.generated.h"

class UImage;

UCLASS(Abstract)
class UCoopArenaEnemyIndicatorWidget : public UUserWidget {
	GENERATED_BODY()
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Ring;

	// enemies farther from the player don't light their sector
	UPROPERTY(EditInstanceOnly, Category="Enemy Indicator", meta=(ClampMin=0))
	float EnemyDistance = 500.0f;

	UPROPERTY(EditInstanceOnly, Category="Enemy Indicator", meta=(ClampMin=0, ClampMax=120))
	float SectorAngleDegree = 80.0f;
};
