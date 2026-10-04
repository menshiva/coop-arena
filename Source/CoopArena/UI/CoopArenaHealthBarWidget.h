#pragma once

#include "Blueprint/UserWidget.h"
#include "CoopArenaHealthBarWidget.generated.h"

class UProgressBar;

UCLASS(Abstract, meta=(DisableNativeTick))
class UCoopArenaHealthBarWidget : public UUserWidget {
	GENERATED_BODY()
public:
	void SetProgress(float Progress) const;
protected:
	virtual void NativePreConstruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> Bar;

	UPROPERTY(EditInstanceOnly, Category="Health Bar")
	FLinearColor Color = FLinearColor::White;
};
