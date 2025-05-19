#pragma once
#include "AruTypes.h"
#include "AruFilter_PathToProperty.generated.h"

USTRUCT(BlueprintType, DisplayName="Path To Property")
struct FAruFilter_PathToProperty : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_PathToProperty() override{};
	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FString PathToProperty{"Path.To.Your.Property"};

	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TInstancedStruct<FAruFilter> Filter;
};
