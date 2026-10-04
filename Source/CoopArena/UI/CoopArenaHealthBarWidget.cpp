#include "CoopArenaHealthBarWidget.h"
#include "Components/ProgressBar.h"

void UCoopArenaHealthBarWidget::SetProgress(const float Progress) const {
	if (Bar)
		Bar->SetPercent(Progress);
}

void UCoopArenaHealthBarWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	if (Bar)
		Bar->SetFillColorAndOpacity(Color);
}
