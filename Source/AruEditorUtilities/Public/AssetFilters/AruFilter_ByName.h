#pragma once
#include "AruTypes.h"
#include "AruFilter_ByName.generated.h"

USTRUCT(BlueprintType, DisplayName="Property Name")
struct FAruFilter_ByName : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByName() override {};
	virtual bool IsConditionMet(FProperty* InProperty, void* InContainer, void* InValue) override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FString PropertyName{"None"};
};
