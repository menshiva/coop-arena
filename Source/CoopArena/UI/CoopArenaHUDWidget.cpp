#include "CoopArenaHUDWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"

void UCoopArenaHUDWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (PlayerAnchor) {
		const auto PawnPtr = GetOwningPlayerPawn();
		const auto SlotPtr = Cast<UCanvasPanelSlot>(PlayerAnchor->Slot);

		FVector2D Center;
		const bool bOnScreen = PawnPtr && SlotPtr && UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
			GetOwningPlayer(), PawnPtr->GetActorLocation(), Center, false
		);

		if (bOnScreen)
			SlotPtr->SetPosition(Center);

		PlayerAnchor->SetVisibility(bOnScreen ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
}
