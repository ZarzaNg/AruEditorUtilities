#pragma once
#include "AruTypes.h"
#include "AruPredicate_Map.generated.h"

USTRUCT(BlueprintType, DisplayName="Add Pair to Map")
struct FAruPredicate_AddMapPair : public FAruPredicate
{
	GENERATED_BODY()
public:
	virtual ~FAruPredicate_AddMapPair() override {};
	virtual void Execute(FProperty* InProperty, void* InContainer, void* InValue) const override;
protected:
	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruPredicate>> PredicatesForKey;

	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruPredicate>> PredicatesForValue;
};

USTRUCT(BlueprintType, DisplayName="Remove Pair(s) from Map")
struct FAruPredicate_RemoveMapPair : public FAruPredicate
{
	GENERATED_BODY()
public:
	virtual ~FAruPredicate_RemoveMapPair() override {};
	virtual void Execute(FProperty* InProperty, void* InContainer, void* InValue) const override;
protected:
	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruFilter>> KeyFilters;

	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruFilter>> ValueFilters;
};

USTRUCT(BlueprintType, DisplayName="Modify Pair(s) from Map")
struct FAruPredicate_ModifyMapPair : public FAruPredicate
{
	GENERATED_BODY()
public:
	virtual ~FAruPredicate_ModifyMapPair() override {};
	virtual void Execute(FProperty* InProperty, void* InContainer, void* InValue) const override;
protected:
	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruFilter>> KeyFilters;

	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruFilter>> ValueFilters;

	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruPredicate>> PredicatesForKey;

	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruPredicate>> PredicatesForValue;
};


