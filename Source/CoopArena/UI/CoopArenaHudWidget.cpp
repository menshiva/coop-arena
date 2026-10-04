#include "CoopArenaHudWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"

void UCoopArenaHudWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (PlayerAnchor) {
		FVector2D Center;
		const auto PawnPtr = GetOwningPlayerPawn();
		const auto SlotPtr = Cast<UCanvasPanelSlot>(PlayerAnchor->Slot);
		const bool bOnScreen = PawnPtr && SlotPtr && UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
			GetOwningPlayer(), PawnPtr->GetActorLocation(), Center, false
		);

		if (bOnScreen)
			SlotPtr->SetPosition(Center);

		PlayerAnchor->SetVisibility(bOnScreen ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
}
