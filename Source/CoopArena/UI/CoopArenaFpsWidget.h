#pragma once

#include "Blueprint/UserWidget.h"
#include "CoopArenaFpsWidget.generated.h"

UCLASS(Abstract)
class UCoopArenaFpsWidget : public UUserWidget {
	GENERATED_BODY()
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category="FPS")
	void OnFrameStatsChanged(int32 Fps, float FrameTimeMs);
private:
	int32 CachedFps = INDEX_NONE;
	int32 CachedFrameMs = INDEX_NONE;
};
