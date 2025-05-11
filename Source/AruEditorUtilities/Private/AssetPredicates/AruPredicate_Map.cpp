#include "AssetPredicates/AruPredicate_Map.h"
#define LOCTEXT_NAMESPACE "FAruEditorUtilitiesModule"

void FAruPredicate_AddMapPair::Execute(FProperty* InProperty, void* InValue) const
{
	const FMapProperty* MapProperty = CastField<FMapProperty>(InProperty);
	if(MapProperty == nullptr)
	{
		return;
	}

	if(PredicatesForKey.Num() == 0)
	{
		return;
	}

	FProperty* KeyProperty = MapProperty->KeyProp;
	FProperty* ValueProperty = MapProperty->ValueProp;
	if(KeyProperty == nullptr || ValueProperty == nullptr)
	{
		return;
	}

	void* PendingKeyPtr = FMemory::Malloc(KeyProperty->GetSize());
	if(PendingKeyPtr == nullptr)
	{
		return;
	}

	ON_SCOPE_EXIT
	{
		KeyProperty->DestroyValue(PendingKeyPtr);
		FMemory::Free(PendingKeyPtr);
	};

	KeyProperty->InitializeValue(PendingKeyPtr);
	for(auto& Predicate : PredicatesForKey)
	{
		if(const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>())
		{
			PredicatePtr->Execute(KeyProperty, PendingKeyPtr);
		}
	}

	FScriptMapHelper MapHelper{MapProperty, InValue};
	if(MapHelper.FindMapPairIndexFromHash(PendingKeyPtr) != INDEX_NONE)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Duplicate keys", "The key pending to add already exists in this map."));
		return;
	}
	
	int32 NewElementIndex = MapHelper.AddDefaultValue_Invalid_NeedsRehash();
	if(!MapHelper.IsValidIndex(NewElementIndex))
	{
		return;
	}
	
	ON_SCOPE_EXIT
	{
		MapHelper.Rehash();
	};
	
	void* NewKeyPtr = MapHelper.GetKeyPtr(NewElementIndex);
	if(NewKeyPtr == nullptr)
	{
		return;
	}
	KeyProperty->CopyCompleteValue(NewKeyPtr, PendingKeyPtr);
	
	void* NewValuePtr = MapHelper.GetValuePtr(NewElementIndex);
	if(NewValuePtr == nullptr)
	{
		return;
	}
	
	for(auto& Predicate : PredicatesForValue)
	{
		if(const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>())
		{
			PredicatePtr->Execute(ValueProperty, NewValuePtr);
		}
	}
}

void FAruPredicate_RemoveMapPair::Execute(FProperty* InProperty, void* InValue) const
{
	if(KeyFilters.Num() == 0 && ValueFilters.Num() == 0)
	{
		return;
	}
	
	const FMapProperty* MapProperty = CastField<FMapProperty>(InProperty);
	if(MapProperty == nullptr)
	{
		return;
	}

	auto ShouldRemove = [&](void* KeyPtr, void* ValuePtr)
	{
		for(const TInstancedStruct<FAruFilter>& FilterStruct : KeyFilters)
		{
			const FAruFilter* Filter = FilterStruct.GetPtr<FAruFilter>();
			if(Filter == nullptr)
			{
				continue;
			}

			if(!Filter->IsConditionMet(MapProperty->KeyProp, KeyPtr))
			{
				return false;
			}
		}

		for(const TInstancedStruct<FAruFilter>& FilterStruct : ValueFilters)
		{
			const FAruFilter* Filter = FilterStruct.GetPtr<FAruFilter>();
			if(Filter == nullptr)
			{
				continue;
			}

			if(!Filter->IsConditionMet(MapProperty->ValueProp, ValuePtr))
			{
				return false;
			}
		}
		
		return true;
	};

	TArray<int32> PendingRemove;
	FScriptMapHelper MapHelper{MapProperty, InValue};
	
	ON_SCOPE_EXIT
	{
		MapHelper.Rehash();
	};
	
	for(int32 Index = 0; Index < MapHelper.Num(); ++Index)  
	{         
		void* MapKeyPtr = MapHelper.GetKeyPtr(Index);  
		void* MapValuePtr = MapHelper.GetValuePtr(Index);

		if(ShouldRemove(MapKeyPtr, MapValuePtr))
		{
			PendingRemove.Add(Index);
		}
	}

	for(int32& Index : PendingRemove)
	{
		MapHelper.RemoveAt(Index);
	}
}

void FAruPredicate_ModifyMapPair::Execute(FProperty* InProperty, void* InValue) const
{
	if(KeyFilters.Num() == 0 && ValueFilters.Num() == 0)
	{
		return;
	}
	
	const FMapProperty* MapProperty = CastField<FMapProperty>(InProperty);
	if(MapProperty == nullptr)
	{
		return;
	}
	
	FProperty* KeyProperty = MapProperty->KeyProp;
	FProperty* ValueProperty = MapProperty->ValueProp;
	if(KeyProperty == nullptr || ValueProperty == nullptr)
	{
		return;
	}

	auto ShouldModify = [&](void* KeyPtr, void* ValuePtr)
	{
		for(const TInstancedStruct<FAruFilter>& FilterStruct : KeyFilters)
		{
			const FAruFilter* Filter = FilterStruct.GetPtr<FAruFilter>();
			if(Filter == nullptr)
			{
				continue;
			}

			if(!Filter->IsConditionMet(MapProperty->KeyProp, KeyPtr))
			{
				return false;
			}
		}

		for(const TInstancedStruct<FAruFilter>& FilterStruct : ValueFilters)
		{
			const FAruFilter* Filter = FilterStruct.GetPtr<FAruFilter>();
			if(Filter == nullptr)
			{
				continue;
			}

			if(!Filter->IsConditionMet(MapProperty->ValueProp, ValuePtr))
			{
				return false;
			}
		}
		
		return true;
	};

	TArray<int32> PendingToModify;
	FScriptMapHelper MapHelper{MapProperty, InValue};
	
	ON_SCOPE_EXIT
	{
		MapHelper.Rehash();
	};
	
	for(int32 Index = 0; Index < MapHelper.Num(); ++Index)
	{
		void* MapKeyPtr = MapHelper.GetKeyPtr(Index);  
		void* MapValuePtr = MapHelper.GetValuePtr(Index);

		if(ShouldModify(MapKeyPtr, MapValuePtr))
		{
			PendingToModify.Add(Index);
		}
	}

	for(int32& Index : PendingToModify)
	{
		void* MapKeyPtr = MapHelper.GetKeyPtr(Index);
		void* PendingKeyPtr = FMemory::Malloc(KeyProperty->GetSize());
		if(PendingKeyPtr == nullptr)
		{
			continue;
		}

		ON_SCOPE_EXIT
		{
			KeyProperty->DestroyValue(PendingKeyPtr);
			FMemory::Free(PendingKeyPtr);
		};

		KeyProperty->CopyCompleteValue(PendingKeyPtr, MapKeyPtr);
		for(auto& Predicate : PredicatesForKey)
		{
			if(const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>())
			{
				PredicatePtr->Execute(MapProperty->KeyProp, PendingKeyPtr);
			}
		}

		if(MapHelper.FindMapPairIndexFromHash(PendingKeyPtr) != INDEX_NONE)
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Duplicate keys", "The modified key already exists in this map."));
			continue;
		}
		KeyProperty->CopyCompleteValue(MapKeyPtr, PendingKeyPtr);

		void* MapValuePtr = MapHelper.GetValuePtr(Index);
		for(auto& Predicate : PredicatesForValue)
		{
			if(const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>())
			{
				PredicatePtr->Execute(MapProperty->ValueProp, MapValuePtr);
			}
		}
		
		MapHelper.Rehash();
	}
}
#undef LOCTEXT_NAMESPACE