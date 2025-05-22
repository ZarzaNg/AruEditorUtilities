#pragma once
#include "AruTypes.h"
#include "AruPredicate_Set.generated.h"

USTRUCT(BlueprintType, DisplayName="Add Element to Set")
struct FAruPredicate_AddSetElement : public  FAruPredicate
{
	GENERATED_BODY()
public:
	virtual ~FAruPredicate_AddSetElement() override {};
	virtual bool Execute(const FProperty* InProperty, void* InValue,
	                     const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruPredicate>> Predicates;
};

USTRUCT(BlueprintType, DisplayName="Remove Element(s) from Set")
struct FAruPredicate_RemoveSetValue : public FAruPredicate
{
	GENERATED_BODY()
public:
	virtual ~FAruPredicate_RemoveSetValue() override {};
	virtual bool Execute(const FProperty* InProperty, void* InValue,
	                     const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruFilter>> Filters;
};

USTRUCT(BlueprintType, DisplayName="Modify Element(s) from Set")
struct FAruPredicate_ModifySetValue : public FAruPredicate
{
	GENERATED_BODY()
public:
	virtual ~FAruPredicate_ModifySetValue() override{};
	virtual bool Execute(const FProperty* InProperty, void* InValue,
	                     const FInstancedPropertyBag& InParameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruFilter>> Filters;
	
	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruPredicate>> Predicates;
};