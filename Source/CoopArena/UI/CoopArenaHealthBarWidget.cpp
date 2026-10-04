#include "CoopArenaHealthBarWidget.h"
#include "Components/ProgressBar.h"

void UCoopArenaHealthBarWidget::NativePreConstruct() {
	Super::NativePreConstruct();

	if (Bar)
		Bar->SetFillColorAndOpacity(Color);
}

void UCoopArenaHealthBarWidget::SetProgress(const float Progress) const {
	if (Bar)
		Bar->SetPercent(Progress);
}
