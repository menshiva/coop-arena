#pragma once

#include "BehaviorTree/Services/BTService_DefaultFocus.h"
#include "CoopArenaBTService_Focus.generated.h"

// custom focus so that enemy would look at the player instead of along the arc
UCLASS()
class UCoopArenaBTService_Focus : public UBTService_DefaultFocus {
	GENERATED_BODY()
public:
	UCoopArenaBTService_Focus();
};
