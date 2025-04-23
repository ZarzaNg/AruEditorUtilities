#pragma once

#include "StructUtils/InstancedStruct.h"
#include "AruTypes.generated.h"

USTRUCT(BlueprintType)
struct FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter(){}
	virtual bool IsConditionMet(FProperty* InProperty, void* InContainer, void* InValue) const {return bInverseCondition;}
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Config, meta=(AdvancedClassDisplay))
	bool bInverseCondition = false;
};

USTRUCT(BlueprintType)
struct FAruPredicate
{
	GENERATED_BODY()
public:
	virtual ~FAruPredicate(){};
	virtual void Execute(FProperty* InProperty, void* InContainer, void* InValue) const {};
};

USTRUCT(BlueprintType)
struct FAruActionDefinition
{
	GENERATED_BODY()
public:
	void Invoke(FProperty* InProperty, void* InContainer, void* InValue);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Aru Editor Utilities", meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruFilter>> ActionConditions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Aru Editor Utilities", meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruPredicate>> ActionPredicates;

	template<typename StructType>
	struct TStructIterator
	{
		int32 Index;
		TArray<TInstancedStruct<StructType>>& StructArrayRef;

		FORCEINLINE TStructIterator(int32 InIndex, TArray<TInstancedStruct<StructType>>& InStructArray)
					: Index(InIndex), StructArrayRef(InStructArray) {};

		FORCEINLINE bool operator!=(const TStructIterator& Other) const {return Index != Other.Index;}
		FORCEINLINE StructType& operator*() const {return StructArrayRef[Index].GetMutable<StructType>();}
		FORCEINLINE TStructIterator& operator++() { ++Index; return *this; }
	};

	template<typename StructType>
	struct TRangedForStructArray
	{
		TArray<TInstancedStruct<StructType>>& StructArrayRef;

		explicit TRangedForStructArray(TArray<TInstancedStruct<StructType>>& InStructArray) : StructArrayRef(InStructArray) {}

		FORCEINLINE TStructIterator<StructType> begin() { return TStructIterator<StructType>(0, StructArrayRef); }
		FORCEINLINE TStructIterator<StructType> end()	{ return TStructIterator<StructType>(StructArrayRef.Num(), StructArrayRef); }
	};

public:
	FORCEINLINE TRangedForStructArray<FAruFilter> ForEachCondition()
	{
		return TRangedForStructArray(ActionConditions);
	}
	FORCEINLINE TRangedForStructArray<FAruPredicate> ForEachPredicates()
	{
		return TRangedForStructArray(ActionPredicates);
	}
};
