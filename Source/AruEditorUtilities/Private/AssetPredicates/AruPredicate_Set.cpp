#include "AssetPredicates/AruPredicate_Set.h"

#define LOCTEXT_NAMESPACE "FAruEditorUtilitiesModule"
void FAruPredicate_AddSetElement::Execute(const FProperty* InProperty, void* InValue) const
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
	
	FProperty* ElementProperty = SetProperty->ElementProp;
	if(ElementProperty == nullptr)
	{
		return;
	}

	void* PendingElementPtr = FMemory::Malloc(ElementProperty->GetSize());
	if(PendingElementPtr == nullptr)
	{
		return;
	}

	ON_SCOPE_EXIT
	{
		ElementProperty->DestroyValue(PendingElementPtr);
		FMemory::Free(PendingElementPtr);
	};
	
	ElementProperty->InitializeValue(PendingElementPtr);
	for(auto& Predicate : Predicates)
	{
		const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>();
		if(PredicatePtr == nullptr)
		{
			continue;
		}
		
		PredicatePtr->Execute(ElementProperty, PendingElementPtr);
	}
	
	FScriptSetHelper SetHelper(SetProperty, InValue);
	if(SetHelper.FindElementIndex(PendingElementPtr) != INDEX_NONE)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Duplicate values", "The value pending to add already exists in this set."));
		return;
	}
	
	int32 NewElementIndex = SetHelper.AddDefaultValue_Invalid_NeedsRehash();
	ON_SCOPE_EXIT
	{
		SetHelper.Rehash();
	};
	
	void* NewElementPtr = SetHelper.GetElementPtr(NewElementIndex);
	if(NewElementPtr == nullptr)
	{
		SetHelper.RemoveAt(NewElementIndex);
		return;
	}

	ElementProperty->CopyCompleteValue(NewElementPtr, PendingElementPtr);
}

void FAruPredicate_RemoveSetValue::Execute(const FProperty* InProperty, void* InValue) const
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
	
	auto ShouldRemove = [&](const void* ValuePtr)
	{
		for(const TInstancedStruct<FAruFilter>& FilterStruct : Filters)
		{
			const FAruFilter* Filter = FilterStruct.GetPtr<FAruFilter>();
			if(Filter == nullptr)
			{
				continue;
			}

			if(!Filter->IsConditionMet(SetProperty->ElementProp, ValuePtr))
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

void FAruPredicate_ModifySetValue::Execute(const FProperty* InProperty, void* InValue) const
{
	const FSetProperty* SetProperty = CastField<FSetProperty>(InProperty);
	if(SetProperty == nullptr)
	{
		return;
	}

	const FProperty* ElementProperty = SetProperty->ElementProp;
	if(ElementProperty == nullptr)
	{
		return;
	}

	if(Filters.Num() == 0)
	{
		return;
	}
	
	auto ShouldModify = [&](const void* ValuePtr)
	{
		for(const TInstancedStruct<FAruFilter>& FilterStruct : Filters)
		{
			const FAruFilter* Filter = FilterStruct.GetPtr<FAruFilter>();
			if(Filter == nullptr)
			{
				continue;
			}

			if(!Filter->IsConditionMet(SetProperty->ElementProp, ValuePtr))
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
		if(!ShouldModify(ElementPtr))
		{
			continue;
		}

		void* PendingElementPtr = FMemory::Malloc(ElementProperty->GetSize());
		if(PendingElementPtr == nullptr)
		{
			return;
		}
		
		ON_SCOPE_EXIT
		{
			ElementProperty->DestroyValue(PendingElementPtr);
			FMemory::Free(PendingElementPtr);
		};
		
		ElementProperty->CopyCompleteValue(PendingElementPtr, ElementPtr);

		for(const TInstancedStruct<FAruPredicate>& PredicateStruct : Predicates)
		{
			const FAruPredicate* Predicate = PredicateStruct.GetPtr<FAruPredicate>();
			if(Predicate == nullptr)
			{
				continue;
			}

			Predicate->Execute(SetProperty->ElementProp, PendingElementPtr);
		}
		
		if(SetHelper.FindElementIndex(PendingElementPtr) != INDEX_NONE)
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Duplicate values", "The modified value already exists in this set."));
			continue;
		}

		ElementProperty->CopyCompleteValue(ElementPtr, PendingElementPtr);
		SetHelper.Rehash();
	}
}

#undef LOCTEXT_NAMESPACE