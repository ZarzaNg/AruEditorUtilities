#pragma once
#include "AruTypes.h"
#include "AruFilter_ByName.generated.h"


UENUM(BlueprintType)
enum class EAruNameCompareOp : uint8
{
	MatchAny,
	MatchAll
};

USTRUCT(BlueprintType, DisplayName="Does Property Name Match")
struct FAruFilter_ByName : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByName() override {};
	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	EAruNameCompareOp CompareOp = EAruNameCompareOp::MatchAny;
	
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FString PropertyName{"None"};
};

USTRUCT(BlueprintType, DisplayName="Does Object Name Match")
struct FAruFilter_ByObjectName : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByObjectName() override {};
	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	EAruNameCompareOp CompareOp = EAruNameCompareOp::MatchAny;
	
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FString ObjectName{"None"};
};
