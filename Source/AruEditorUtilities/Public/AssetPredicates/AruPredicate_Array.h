#pragma once
#include "AruTypes.h"
#include "AruPredicate_Array.generated.h"

USTRUCT(BlueprintType, DisplayName="Add Value to Array")
struct FAruPredicate_AddArrayValue : public FAruPredicate
{
	GENERATED_BODY()
public:
	virtual ~FAruPredicate_AddArrayValue() override {};
	virtual void Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruPredicate>> Predicates;
};


USTRUCT(BlueprintType, DisplayName="Remove Value(s) from Array")
struct FAruPredicate_RemoveArrayValue : public FAruPredicate
{
	GENERATED_BODY()
public:
	virtual ~FAruPredicate_RemoveArrayValue() override {};
	virtual void Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruFilter>> Filters;
};

USTRUCT(BlueprintType, DisplayName="Modify Value(s) from Array")
struct FAruPredicate_ModifyArrayValue : public FAruPredicate
{
	GENERATED_BODY()
public:
	virtual ~FAruPredicate_ModifyArrayValue() override{};
	virtual void Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruFilter>> Filters;
	
	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruPredicate>> Predicates;
};
