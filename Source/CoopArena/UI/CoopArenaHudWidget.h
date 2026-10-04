#pragma once

#include "Blueprint/UserWidget.h"
#include "CoopArenaHudWidget.generated.h"

class UCoopArenaHealthWidget;

UCLASS(Abstract)
class UCoopArenaHudWidget : public UUserWidget {
	GENERATED_BODY()
public:
	FORCEINLINE UCoopArenaHealthWidget* GetHealthWidget() const { return HealthWidget; }
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidget> PlayerAnchor;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCoopArenaHealthWidget> HealthWidget;
};
