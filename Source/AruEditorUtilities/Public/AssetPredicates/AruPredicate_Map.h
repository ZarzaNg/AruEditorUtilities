﻿#pragma once
#include "AruTypes.h"
#include "AruPredicate_Map.generated.h"

USTRUCT(BlueprintType, DisplayName="Add Pair to Map")
struct FAruPredicate_AddMapPair : public FAruPredicate
{
	GENERATED_BODY()

public:
	virtual ~FAruPredicate_AddMapPair() override {};

	virtual bool Execute(
		const FProperty* InProperty,
		void* InValue,
		const FInstancedPropertyBag& InParameters) const override;

protected:
	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruPredicate>> PredicatesForKey;

	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruPredicate>> PredicatesForValue;
private:
	static FString GetCompactName() { return {"AddToMap"}; }
};

USTRUCT(BlueprintType, DisplayName="Remove Pair(s) from Map")
struct FAruPredicate_RemoveMapPair : public FAruPredicate
{
	GENERATED_BODY()

public:
	virtual ~FAruPredicate_RemoveMapPair() override {};

	virtual bool Execute(
		const FProperty* InProperty,
		void* InValue,
		const FInstancedPropertyBag& InParameters) const override;

protected:
	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruFilter>> KeyFilters;

	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruFilter>> ValueFilters;
private:
	static FString GetCompactName() { return {"RemoveFromMap"}; }
};

USTRUCT(BlueprintType, DisplayName="Modify Pair(s) from Map")
struct FAruPredicate_ModifyMapPair : public FAruPredicate
{
	GENERATED_BODY()

public:
	virtual ~FAruPredicate_ModifyMapPair() override {};

	virtual bool Execute(
		const FProperty* InProperty,
		void* InValue,
		const FInstancedPropertyBag& InParameters) const override;

protected:
	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruFilter>> KeyFilters;

	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruFilter>> ValueFilters;

	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruPredicate>> PredicatesForKey;

	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruPredicate>> PredicatesForValue;
private:
	static FString GetCompactName() { return {"ModifyMap"}; }
};
