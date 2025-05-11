#pragma once
#include "AruTypes.h"
#include "GameplayTagContainer.h"
#include "AruFilter_ByValue.generated.h"

UENUM(BlueprintType)
enum class EAruNumericCompareOp : uint8
{
	Equip,
	NotEqual,
	GreaterThan,
	LessThan
};

UENUM(BlueprintType)
enum class EAruBooleanCompareOp : uint8
{
	Is,
	Not
};

UENUM(BlueprintType)
enum class EAruContainerCompareOp : uint8
{
	HasAny,
	HasAll
};

USTRUCT(BlueprintType, DisplayName="Numeric")
struct FAruFilter_ByNumericValue : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByNumericValue() override {};
	virtual bool IsConditionMet(FProperty* InProperty, void* InValue) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	float ConditionValue = 0.f;

	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	EAruNumericCompareOp CompareOp = EAruNumericCompareOp::Equip;
};

USTRUCT(BlueprintType, DisplayName="Boolean")
struct FAruFilter_ByBoolean : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByBoolean() override {};
	virtual bool IsConditionMet(FProperty* InProperty, void* InValue) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	bool ConditionValue = false;

	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	EAruBooleanCompareOp CompareOp = EAruBooleanCompareOp::Is;
};

USTRUCT(BlueprintType, DisplayName="Object")
struct FAruFilter_ByObject : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByObject() override {};
	virtual bool IsConditionMet(FProperty* InProperty, void* InValue) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	TObjectPtr<UObject> ConditionValue = nullptr;
	
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	EAruBooleanCompareOp CompareOp = EAruBooleanCompareOp::Is;
};

USTRUCT(BlueprintType, DisplayName="Gameplay Tag")
struct FAruFilter_ByGameplayTagContainer : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByGameplayTagContainer() override {};
	virtual bool IsConditionMet(FProperty* InProperty, void* InValue) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FGameplayTagQuery TagQuery;
};
