#pragma once

#include "StructUtils/InstancedStruct.h"
#include "StructUtils/PropertyBag.h"
#include "AruTypes.generated.h"

USTRUCT(BlueprintType)
struct FAruFilter
{
	GENERATED_BODY()
public:
	virtual ~FAruFilter(){}
	virtual bool IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const {return bInverseCondition;}
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
	virtual void Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const {};
};

USTRUCT(BlueprintType)
struct FAruActionDefinition
{
	GENERATED_BODY()
public:
	void Invoke(FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Aru Editor Utilities", meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruFilter>> ActionConditions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Aru Editor Utilities", meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FAruPredicate>> ActionPredicates;

	template<typename StructType>
	struct TStructIterator
	{
		int32 Index;
		const TArray<TInstancedStruct<StructType>>& StructArrayRef;

		FORCEINLINE TStructIterator(int32 InIndex, const TArray<TInstancedStruct<StructType>>& InStructArray)
					: Index(InIndex), StructArrayRef(InStructArray) {};

		FORCEINLINE bool operator!=(const TStructIterator& Other) const {return Index != Other.Index;}
		FORCEINLINE const StructType& operator*() const {return StructArrayRef[Index].Get<const StructType>();}
		FORCEINLINE TStructIterator& operator++() { ++Index; return *this; }
	};

	template<typename StructType>
	struct TRangedForStructArray
	{
		const TArray<TInstancedStruct<StructType>>& StructArrayRef;

		explicit TRangedForStructArray(const TArray<TInstancedStruct<StructType>>& InStructArray) : StructArrayRef(InStructArray) {}

		FORCEINLINE TStructIterator<StructType> begin() { return TStructIterator<StructType>(0, StructArrayRef); }
		FORCEINLINE TStructIterator<StructType> end()	{ return TStructIterator<StructType>(StructArrayRef.Num(), StructArrayRef); }
	};

public:
	FORCEINLINE TRangedForStructArray<FAruFilter> ForEachCondition() const
	{
		return TRangedForStructArray{ActionConditions};
	}
	FORCEINLINE TRangedForStructArray<FAruPredicate> ForEachPredicates() const
	{
		return TRangedForStructArray{ActionPredicates};
	}
};

UCLASS(BlueprintType)
class ARUEDITORUTILITIES_API UAruActionConfigData : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FAruActionDefinition> ActionDefinitions;
};

USTRUCT(BlueprintType)
struct FAruProcessConfig
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	FInstancedPropertyBag Parameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSearchDepth = 5;
};


