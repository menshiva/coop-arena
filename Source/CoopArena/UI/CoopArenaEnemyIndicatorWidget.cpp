#include "CoopArenaEnemyIndicatorWidget.h"
#include "EngineUtils.h"
#include "Character/CoopArenaEnemyCharacter.h"
#include "Components/Image.h"

void UCoopArenaEnemyIndicatorWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!Ring)
		return;

	const auto PawnPtr = GetOwningPlayerPawn();
	const auto CameraManagerPtr = GetOwningPlayerCameraManager();
	const auto MaterialPtr = Ring->GetDynamicMaterial();
	if (!PawnPtr || !CameraManagerPtr || !MaterialPtr) {
		Ring->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	auto HideValues = FVector::OneVector;
	bool bAnyOccupied = false;
	{
		// which back sectors have an enemy nearby (relative to the camera)
		const auto PawnPos = PawnPtr->GetActorLocation();
		const FRotator CameraYaw(0.0, CameraManagerPtr->GetCameraRotation().Yaw, 0.0);
		const double MaxDistSq = FMath::Square(EnemyDistance);
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
	}

	if (bAnyOccupied) {
		static const FName HideParam(TEXT("HideValues"));
		MaterialPtr->SetVectorParameterValue(HideParam, HideValues);

		static const FName ArcHalfAngleParam(TEXT("ArcHalfAngle"));
		MaterialPtr->SetScalarParameterValue(ArcHalfAngleParam, 1.5f * SectorAngleDegree);
	}

	Ring->SetVisibility(bAnyOccupied ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}
