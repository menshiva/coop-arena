#include "CoopArenaFpsWidget.h"

// what `stat fps` shows
extern ENGINE_API float GAverageFPS;
extern ENGINE_API float GAverageMS;

void UCoopArenaFpsWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);

	const int32 Fps = FMath::RoundToInt(GAverageFPS);
	const int32 FrameMs = FMath::RoundToInt(GAverageMS * 10.0f);
	if (Fps == CachedFps && FrameMs == CachedFrameMs)
		return; // notify only when the shown numbers change

	CachedFps = Fps;
	CachedFrameMs = FrameMs;

	OnFrameStatsChanged(Fps, FrameMs / 10.0f);
}
