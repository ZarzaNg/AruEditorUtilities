#pragma once
#include "AruTypes.h"
#include "AruFilter_ByPath.generated.h"

USTRUCT(BlueprintType, DisplayName="Does Object Path Match")
struct FAruFilter_ByObjectPath : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByObjectPath() override {};
	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly)
	TArray<FString> MatchingContexts{};
};
