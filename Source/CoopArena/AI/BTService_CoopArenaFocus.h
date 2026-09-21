#pragma once

#include "BehaviorTree/Services/BTService_DefaultFocus.h"
#include "BTService_CoopArenaFocus.generated.h"

// custom focus so that enemy would look at the player instead of along the arc
UCLASS()
class UBTService_CoopArenaFocus : public UBTService_DefaultFocus {
	GENERATED_BODY()
public:
	UBTService_CoopArenaFocus();
};
