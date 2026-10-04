#pragma once

#include "Blueprint/UserWidget.h"
#include "CoopArenaHealthWidget.generated.h"

class UCoopArenaHealthBarWidget;

UCLASS(Abstract, meta=(DisableNativeTick))
class UCoopArenaHealthWidget : public UUserWidget {
	GENERATED_BODY()
public:
	void SetHealth(int32 Health, int32 MaxHealth);
protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Health")
	void OnHealthChanged(int32 Health, int32 MaxHealth);

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCoopArenaHealthBarWidget> Bar;
};
