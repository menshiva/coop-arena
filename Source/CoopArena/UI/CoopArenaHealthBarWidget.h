#pragma once

#include "Blueprint/UserWidget.h"
#include "CoopArenaHealthBarWidget.generated.h"

class UProgressBar;

UCLASS(Abstract, meta=(DisableNativeTick))
class UCoopArenaHealthBarWidget : public UUserWidget {
	GENERATED_BODY()
protected:
	virtual void NativePreConstruct() override;
public:
	void SetProgress(float Progress) const;
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> Bar;

	UPROPERTY(EditInstanceOnly, Category="Health Bar")
	FLinearColor Color = FLinearColor::White;
};
