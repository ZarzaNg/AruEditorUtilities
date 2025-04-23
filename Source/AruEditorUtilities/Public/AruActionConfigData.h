#pragma once

#include "CoreMinimal.h"
#include "AruTypes.h"
#include "Engine/DataAsset.h"
#include "AruActionConfigData.generated.h"

UCLASS(BlueprintType)
class ARUEDITORUTILITIES_API UAruActionConfigData : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay, BlueprintReadWrite)
	TArray<FAruActionDefinition> ActionDefinitions;
};