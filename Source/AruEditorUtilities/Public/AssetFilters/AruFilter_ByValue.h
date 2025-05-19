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

USTRUCT(BlueprintType, DisplayName="Is Numeric Value Eligible")
struct FAruFilter_ByNumericValue : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByNumericValue() override {};
	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	float ConditionValue = 0.f;

	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	EAruNumericCompareOp CompareOp = EAruNumericCompareOp::Equip;
};

USTRUCT(BlueprintType, DisplayName="Is Boolean Value Eligible")
struct FAruFilter_ByBoolean : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByBoolean() override {};
	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	bool ConditionValue = false;

	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	EAruBooleanCompareOp CompareOp = EAruBooleanCompareOp::Is;
};

USTRUCT(BlueprintType, DisplayName="Is Object Value Eligible")
struct FAruFilter_ByObject : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByObject() override {};
	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	TObjectPtr<UObject> ConditionValue = nullptr;
	
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	EAruBooleanCompareOp CompareOp = EAruBooleanCompareOp::Is;
};

USTRUCT(BlueprintType, DisplayName="Is Enumeration Value Eligible")
struct FAruFilter_ByEnum : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByEnum() override {};
	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FString ConditionValue{};

	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	EAruBooleanCompareOp CompareOp = EAruBooleanCompareOp::Is;
};

USTRUCT(BlueprintType, DisplayName="Is String Value Eligible")
struct FAruFilter_ByString : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByString() override {};
	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FString ConditionValue{};

	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	EAruContainerCompareOp CompareOp = EAruContainerCompareOp::HasAny;
	
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	bool bCaseSensitive = false;
};

USTRUCT(BlueprintType, DisplayName="Is Text Value Eligible")
struct FAruFilter_ByText : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByText() override {};
	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FString ConditionValue{};

	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	EAruContainerCompareOp CompareOp = EAruContainerCompareOp::HasAny;

	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	bool bCaseSensitive = false;
};

USTRUCT(BlueprintType, DisplayName="Are GameplayTags Eligible")
struct FAruFilter_ByGameplayTagContainer : public FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter_ByGameplayTagContainer() override {};
	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FGameplayTagQuery TagQuery;
};
