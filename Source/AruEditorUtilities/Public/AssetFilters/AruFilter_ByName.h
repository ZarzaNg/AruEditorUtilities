#pragma once
#include "AruTypes.h"
#include "AruFilter_ByName.generated.h"

USTRUCT(BlueprintType, DisplayName="Does Property Name Match")
struct FAruFilter_ByName : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByName() override {};
	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FString PropertyName{"None"};
};
