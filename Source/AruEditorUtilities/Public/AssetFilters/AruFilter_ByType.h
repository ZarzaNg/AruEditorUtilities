#pragma once
#include "AruTypes.h"
#include "AruFilter_ByType.generated.h"

USTRUCT(BlueprintType, DisplayName="Check Object Type")
struct FAruFilter_ByObjectType : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByObjectType() override {}
	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UClass> ObjectType = nullptr;
};

USTRUCT(BlueprintType, DisplayName="Check Struct Type")
struct FAruFilter_ByStructType : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByStructType() override {}
	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UScriptStruct> StructType = nullptr;
};

USTRUCT(BlueprintType, DisplayName="Check Instanced Struct Type")
struct FAruFilter_ByInstancedStructType : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByInstancedStructType() override {}
	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UScriptStruct> StructType = nullptr;
};
