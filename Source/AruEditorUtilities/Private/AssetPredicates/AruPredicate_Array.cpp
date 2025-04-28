#include "AssetPredicates/AruPredicate_Array.h"

void FAruPredicate_AddArrayValue::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
{
	const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(InProperty);
	if(ArrayProperty == nullptr)
	{
		return;
	}

	if(Predicates.Num() == 0)
	{
		return;
	}
	
	FScriptArrayHelper ArrayHelper{ArrayProperty, InValue};
	int32 NewElementIndex = ArrayHelper.AddValue();
	if(!ArrayHelper.IsValidIndex(NewElementIndex))
	{
		return;
	}
	
	FProperty* ElementProperty = ArrayProperty->Inner;
	void* NewElementPtr = ArrayHelper.GetRawPtr(NewElementIndex);
	for(auto& Predicate : Predicates)
	{
		const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>();
		if(PredicatePtr == nullptr)
		{
			continue;
		}
		
		PredicatePtr->Execute(ElementProperty, InContainer, NewElementPtr);
	}
}

void FAruPredicate_RemoveArrayValue::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
{
	const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(InProperty);
	if(ArrayProperty == nullptr)
	{
		return;
	}

	if(Filters.Num() == 0)
	{
		return;
	}

	auto ShouldRemove = [&](void* ValuePtr)
	{
		for(const TInstancedStruct<FAruFilter>& FilterStruct : Filters)
		{
			const FAruFilter* Filter = FilterStruct.GetPtr<FAruFilter>();
			if(Filter == nullptr)
			{
				continue;
			}

			if(!Filter->IsConditionMet(ArrayProperty->Inner, InContainer, ValuePtr))
			{
				return false;
			}
		}
		return true;
	};

	TArray<int32> PendingRemove;
	FScriptArrayHelper ArrayHelper{ArrayProperty, InValue};
	for(int32 Index = 0; Index < ArrayHelper.Num(); ++Index)  
	{
		if(ShouldRemove(ArrayHelper.GetRawPtr(Index)))
		{
			PendingRemove.Add(Index);
		}
	}

	for(int32& Index : PendingRemove)
	{
		ArrayHelper.RemoveValues(Index);
	}
}

void FAruPredicate_ModifyArrayValue::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
{
	const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(InProperty);
	if(ArrayProperty == nullptr)
	{
		return;
	}

	if(Predicates.Num() == 0)
	{
		return;
	}

	auto ShouldModify = [&](void* ValuePtr)
	{
		for(const TInstancedStruct<FAruFilter>& FilterStruct : Filters)
		{
			const FAruFilter* Filter = FilterStruct.GetPtr<FAruFilter>();
			if(Filter == nullptr)
			{
				continue;
			}

			if(!Filter->IsConditionMet(ArrayProperty->Inner, InContainer, ValuePtr))
			{
				return false;
			}
		}
		return true;
	};

	FScriptArrayHelper ArrayHelper{ArrayProperty, InValue};
	for(int32 Index = 0; Index < ArrayHelper.Num(); ++Index)  
	{
		void* ElementPtr = ArrayHelper.GetRawPtr(Index);
		if(ShouldModify(ElementPtr))
		{
			for(const TInstancedStruct<FAruPredicate>& PredicateStruct : Predicates)
			{
				const FAruPredicate* Predicate = PredicateStruct.GetPtr<FAruPredicate>();
				if(Predicate == nullptr)
				{
					continue;
				}

				Predicate->Execute(ArrayProperty->Inner, InContainer, ElementPtr);
			}
		}
	}
}
