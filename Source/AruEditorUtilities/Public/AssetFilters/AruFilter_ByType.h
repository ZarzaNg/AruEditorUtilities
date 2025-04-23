#pragma once
#include "AruTypes.h"
#include "AruFilter_ByType.generated.h"

USTRUCT(BlueprintType, DisplayName="Object Type")
struct FAruFilter_ByObjectType : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByObjectType() override {};
	virtual bool IsConditionMet(FProperty* InProperty, void* InContainer, void* InValue) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	TObjectPtr<UClass> ObjectType = nullptr;;
};

USTRUCT(BlueprintType, DisplayName="Struct Type")
struct FAruFilter_ByStructType : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByStructType() override {};
	virtual bool IsConditionMet(FProperty* InProperty, void* InContainer, void* InValue) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	TObjectPtr<UScriptStruct> StructType = nullptr;
};
