#include "CoopArenaHealthWidget.h"
#include "CoopArenaHealthBarWidget.h"

void UCoopArenaHealthWidget::SetHealth(const int32 Health, const int32 MaxHealth) {
	if (Bar)
		Bar->SetProgress(MaxHealth > 0 ? static_cast<float>(Health) / MaxHealth : 0.0f);
	OnHealthChanged(Health, MaxHealth);
}
