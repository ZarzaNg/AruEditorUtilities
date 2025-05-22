#include "AssetPredicates/AruPredicate_Array.h"

bool FAruPredicate_AddArrayValue::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(InProperty);
	if (ArrayProperty == nullptr)
	{
		return false;
	}

	if (Predicates.Num() == 0)
	{
		return false;
	}

	FScriptArrayHelper ArrayHelper{ArrayProperty, InValue};
	int32 NewElementIndex = ArrayHelper.AddValue();
	if (!ArrayHelper.IsValidIndex(NewElementIndex))
	{
		return false;
	}

	FProperty* ElementProperty = ArrayProperty->Inner;
	void* NewElementPtr = ArrayHelper.GetRawPtr(NewElementIndex);
	for (auto& Predicate : Predicates)
	{
		const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>();
		if (PredicatePtr == nullptr)
		{
			continue;
		}

		PredicatePtr->Execute(ElementProperty, NewElementPtr, InParameters);
	}

	return true;
}

bool FAruPredicate_RemoveArrayValue::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(InProperty);
	if (ArrayProperty == nullptr)
	{
		return false;
	}

	if (Filters.Num() == 0)
	{
		return false;
	}

	auto ShouldRemove = [&](const void* ValuePtr)
	{
		for (const TInstancedStruct<FAruFilter>& FilterStruct : Filters)
		{
			const FAruFilter* Filter = FilterStruct.GetPtr<FAruFilter>();
			if (Filter == nullptr)
			{
				continue;
			}

			if (!Filter->IsConditionMet(ArrayProperty->Inner, ValuePtr, InParameters))
			{
				return false;
			}
		}
		return true;
	};

	TArray<int32> PendingRemove;
	FScriptArrayHelper ArrayHelper{ArrayProperty, InValue};
	for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
	{
		if (ShouldRemove(ArrayHelper.GetRawPtr(Index)))
		{
			PendingRemove.Add(Index);
		}
	}

	for (int32& Index : PendingRemove)
	{
		ArrayHelper.RemoveValues(Index);
	}

	return PendingRemove.Num() > 0;
}

bool FAruPredicate_ModifyArrayValue::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(InProperty);
	if (ArrayProperty == nullptr)
	{
		return false;
	}

	if (Predicates.Num() == 0)
	{
		return false;
	}

	auto ShouldModify = [&](const void* ValuePtr)
	{
		for (const TInstancedStruct<FAruFilter>& FilterStruct : Filters)
		{
			const FAruFilter* Filter = FilterStruct.GetPtr<FAruFilter>();
			if (Filter == nullptr)
			{
				continue;
			}

			if (!Filter->IsConditionMet(ArrayProperty->Inner, ValuePtr, InParameters))
			{
				return false;
			}
		}
		return true;
	};

	FScriptArrayHelper ArrayHelper{ArrayProperty, InValue};
	bool bExecutedSuccessfully = false;
	for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
	{
		void* ElementPtr = ArrayHelper.GetRawPtr(Index);
		if (!ShouldModify(ElementPtr))
		{
			continue;
		}

		for (const TInstancedStruct<FAruPredicate>& PredicateStruct : Predicates)
		{
			const FAruPredicate* Predicate = PredicateStruct.GetPtr<FAruPredicate>();
			if (Predicate == nullptr)
			{
				continue;
			}

			bExecutedSuccessfully |= Predicate->Execute(ArrayProperty->Inner, ElementPtr, InParameters);
		}
	}
	return bExecutedSuccessfully;
}
