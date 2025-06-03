#pragma once
#include "AruTypes.h"
#include "AruFilter_ByPath.generated.h"

USTRUCT(BlueprintType, DisplayName="Check Asset Path")
struct FAruFilter_ByAssetPath : public FAruFilter
{
	GENERATED_BODY()

public:
	virtual ~FAruFilter_ByAssetPath() override {};

	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const override;

protected:
	UPROPERTY(EditDefaultsOnly)
	TArray<FString> MatchingContexts{};
};
