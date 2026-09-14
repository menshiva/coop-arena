#pragma once

#include "Blueprint/UserWidget.h"
#include "CoopArenaHealthWidget.generated.h"

UCLASS(Abstract, meta=(DisableNativeTick))
class UCoopArenaHealthWidget : public UUserWidget {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent, Category="Health")
	void OnHealthChanged(int32 Health, int32 MaxHealth);
};
