#include "CoopArenaEnemyIndicatorWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Enemy/CoopArenaEnemyCharacter.h"
#include "EngineUtils.h"

void UCoopArenaEnemyIndicatorWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);

	const auto PawnPtr = GetOwningPlayerPawn();
	const auto CameraManager = GetOwningPlayerCameraManager();
	const auto Material = Ring->GetDynamicMaterial();
	const auto SlotPtr = Cast<UCanvasPanelSlot>(Ring->Slot);
	if (!PawnPtr || !CameraManager || !Material || !SlotPtr) {
		Ring->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	FVector2D Center;
	if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayer(), PawnPtr->GetActorLocation(), Center, false)) {
		Ring->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	const auto PawnPos = PawnPtr->GetActorLocation();
	const FRotator CameraYaw(0.0, CameraManager->GetCameraRotation().Yaw, 0.0);
	const double MaxDistSq = FMath::Square(EnemyDistance);
	auto HideValues = FVector::OneVector;
	bool bAnyOccupied = false;
	for (TActorIterator<ACoopArenaEnemyCharacter> It(GetWorld()); It; ++It) {
		const auto ToEnemy = It->GetActorLocation() - PawnPos;
		if (ToEnemy.SizeSquared() > MaxDistSq)
			continue;
		const auto Local = CameraYaw.UnrotateVector(ToEnemy);

		const double Bearing = FMath::RadiansToDegrees(FMath::Atan2(FMath::Abs(Local.Y), Local.X));
		if (Bearing >= 180.0 - 0.5 * SectorAngleDegree) {
			HideValues[1] = 0.0; // back
		}
		else if (Bearing >= 180.0 - 1.5 * SectorAngleDegree) {
			if (Local.Y > 0.0)
				HideValues[0] = 0.0; // right
			else
				HideValues[2] = 0.0; // left
		}
		else {
			continue;
		}

		bAnyOccupied = true;
	}

	if (bAnyOccupied) {
		static const FName HideParam(TEXT("HideValues"));
		Material->SetVectorParameterValue(HideParam, HideValues);

		static const FName ArcHalfAngleParam(TEXT("ArcHalfAngle"));
		Material->SetScalarParameterValue(ArcHalfAngleParam, 1.5f * SectorAngleDegree);

		SlotPtr->SetPosition(Center);
	}

	Ring->SetVisibility(bAnyOccupied ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}
