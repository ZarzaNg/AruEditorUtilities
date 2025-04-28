#include "AssetPredicates/AruPredicate_Set.h"

void FAruPredicate_AddSetElement::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
{
	const FSetProperty* SetProperty = CastField<FSetProperty>(InProperty);
	if(SetProperty == nullptr)
	{
		return;
	}

	if(Predicates.Num() == 0)
	{
		return;
	}
	
	FScriptSetHelper SetHelper(SetProperty, InValue);
	int32 NewElementIndex = SetHelper.AddDefaultValue_Invalid_NeedsRehash();

	ON_SCOPE_EXIT
	{
		SetHelper.Rehash();
	};
	
	if(!SetHelper.IsValidIndex(NewElementIndex))
	{
		return;
	}
	
	FProperty* ElementProperty = SetProperty->ElementProp;
	void* NewElementPtr = SetHelper.GetElementPtr(NewElementIndex);
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

void FAruPredicate_RemoveSetValue::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
{
	const FSetProperty* SetProperty = CastField<FSetProperty>(InProperty);
	if(SetProperty == nullptr)
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

			if(!Filter->IsConditionMet(SetProperty->ElementProp, InContainer, ValuePtr))
			{
				return false;
			}
		}
		return true;
	};

	TArray<int32> PendingRemove;
	FScriptSetHelper SetHelper{SetProperty, InValue};
	for(int32 Index = 0; Index < SetHelper.Num(); ++Index)  
	{
		if(ShouldRemove(SetHelper.GetElementPtr(Index)))
		{
			PendingRemove.Add(Index);
		}
	}

	for(int32& Index : PendingRemove)
	{
		SetHelper.RemoveAt(Index);
	}
}

void FAruPredicate_ModifySetValue::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
{
	const FSetProperty* SetProperty = CastField<FSetProperty>(InProperty);
	if(SetProperty == nullptr)
	{
		return;
	}

	if(Filters.Num() == 0)
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

			if(!Filter->IsConditionMet(SetProperty->ElementProp, InContainer, ValuePtr))
			{
				return false;
			}
		}
		return true;
	};

	FScriptSetHelper SetHelper{SetProperty, InValue};
	for(int32 Index = 0; Index < SetHelper.Num(); ++Index)  
	{
		void* ElementPtr = SetHelper.GetElementPtr(Index);
		if(ShouldModify(ElementPtr))
		{
			for(const TInstancedStruct<FAruPredicate>& PredicateStruct : Predicates)
			{
				const FAruPredicate* Predicate = PredicateStruct.GetPtr<FAruPredicate>();
				if(Predicate == nullptr)
				{
					continue;
				}

				Predicate->Execute(SetProperty->ElementProp, InContainer, ElementPtr);
			}
		}
	}
}
