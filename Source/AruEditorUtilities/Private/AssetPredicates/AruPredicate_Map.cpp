#include "AssetPredicates/AruPredicate_Map.h"
#define LOCTEXT_NAMESPACE "FAruEditorUtilitiesModule"

bool FAruPredicate_AddMapPair::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	const FMapProperty* MapProperty = CastField<FMapProperty>(InProperty);
	if (MapProperty == nullptr)
	{
		return false;
	}

	if (PredicatesForKey.Num() == 0)
	{
		return false;
	}

	FProperty* KeyProperty = MapProperty->KeyProp;
	FProperty* ValueProperty = MapProperty->ValueProp;
	if (KeyProperty == nullptr || ValueProperty == nullptr)
	{
		return false;
	}

	void* PendingKeyPtr = FMemory::Malloc(KeyProperty->GetSize());
	if (PendingKeyPtr == nullptr)
	{
		return false;
	}

	ON_SCOPE_EXIT
	{
		KeyProperty->DestroyValue(PendingKeyPtr);
		FMemory::Free(PendingKeyPtr);
	};

	bool bExecutedSuccessfully = false;

	KeyProperty->InitializeValue(PendingKeyPtr);
	for (auto& Predicate : PredicatesForKey)
	{
		if (const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>())
		{
			bExecutedSuccessfully |= PredicatePtr->Execute(KeyProperty, PendingKeyPtr, InParameters);
		}
	}

	if (bExecutedSuccessfully == false)
	{
		// TODO: Add Log.
		return false;
	}

	FScriptMapHelper MapHelper{MapProperty, InValue};
	if (MapHelper.FindMapPairIndexFromHash(PendingKeyPtr) != INDEX_NONE)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Duplicate keys", "The key pending to add already exists in this map."));
		return false;
	}

	int32 NewElementIndex = MapHelper.AddDefaultValue_Invalid_NeedsRehash();
	if (!MapHelper.IsValidIndex(NewElementIndex))
	{
		MapHelper.RemoveAt(NewElementIndex);
		return false;
	}

	ON_SCOPE_EXIT
	{
		MapHelper.Rehash();
	};

	void* NewKeyPtr = MapHelper.GetKeyPtr(NewElementIndex);
	if (NewKeyPtr == nullptr)
	{
		MapHelper.RemoveAt(NewElementIndex);
		return false;
	}
	KeyProperty->CopyCompleteValue(NewKeyPtr, PendingKeyPtr);

	void* NewValuePtr = MapHelper.GetValuePtr(NewElementIndex);
	if (NewValuePtr == nullptr)
	{
		MapHelper.RemoveAt(NewElementIndex);
		return false;
	}

	for (auto& Predicate : PredicatesForValue)
	{
		if (const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>())
		{
			PredicatePtr->Execute(ValueProperty, NewValuePtr, InParameters);
		}
	}

	return bExecutedSuccessfully;
}

bool FAruPredicate_RemoveMapPair::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (KeyFilters.Num() == 0 && ValueFilters.Num() == 0)
	{
		return false;
	}

	const FMapProperty* MapProperty = CastField<FMapProperty>(InProperty);
	if (MapProperty == nullptr)
	{
		return false;
	}

	auto ShouldRemove = [&](const void* KeyPtr, const void* ValuePtr)
	{
		for (const TInstancedStruct<FAruFilter>& FilterStruct : KeyFilters)
		{
			const FAruFilter* Filter = FilterStruct.GetPtr<FAruFilter>();
			if (Filter == nullptr)
			{
				continue;
			}

			if (!Filter->IsConditionMet(MapProperty->KeyProp, KeyPtr, InParameters))
			{
				return false;
			}
		}

		for (const TInstancedStruct<FAruFilter>& FilterStruct : ValueFilters)
		{
			const FAruFilter* Filter = FilterStruct.GetPtr<FAruFilter>();
			if (Filter == nullptr)
			{
				continue;
			}

			if (!Filter->IsConditionMet(MapProperty->ValueProp, ValuePtr, InParameters))
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

	for (int32 Index = 0; Index < MapHelper.Num(); ++Index)
	{
		void* MapKeyPtr = MapHelper.GetKeyPtr(Index);
		void* MapValuePtr = MapHelper.GetValuePtr(Index);

		if (ShouldRemove(MapKeyPtr, MapValuePtr))
		{
			PendingRemove.Add(Index);
		}
	}

	for (int32& Index : PendingRemove)
	{
		MapHelper.RemoveAt(Index);
	}
	return PendingRemove.Num() > 0;
}

bool FAruPredicate_ModifyMapPair::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (KeyFilters.Num() == 0 && ValueFilters.Num() == 0)
	{
		return false;
	}

	const FMapProperty* MapProperty = CastField<FMapProperty>(InProperty);
	if (MapProperty == nullptr)
	{
		return false;
	}

	FProperty* KeyProperty = MapProperty->KeyProp;
	FProperty* ValueProperty = MapProperty->ValueProp;
	if (KeyProperty == nullptr || ValueProperty == nullptr)
	{
		return false;
	}

	auto ShouldModify = [&](const void* KeyPtr, const void* ValuePtr)
	{
		for (const TInstancedStruct<FAruFilter>& FilterStruct : KeyFilters)
		{
			const FAruFilter* Filter = FilterStruct.GetPtr<FAruFilter>();
			if (Filter == nullptr)
			{
				continue;
			}

			if (!Filter->IsConditionMet(MapProperty->KeyProp, KeyPtr, InParameters))
			{
				return false;
			}
		}

		for (const TInstancedStruct<FAruFilter>& FilterStruct : ValueFilters)
		{
			const FAruFilter* Filter = FilterStruct.GetPtr<FAruFilter>();
			if (Filter == nullptr)
			{
				continue;
			}

			if (!Filter->IsConditionMet(MapProperty->ValueProp, ValuePtr, InParameters))
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

	for (int32 Index = 0; Index < MapHelper.Num(); ++Index)
	{
		void* MapKeyPtr = MapHelper.GetKeyPtr(Index);
		void* MapValuePtr = MapHelper.GetValuePtr(Index);

		if (ShouldModify(MapKeyPtr, MapValuePtr))
		{
			PendingToModify.Add(Index);
		}
	}

	bool bExecutedSuccessfully = false;
	for (int32& Index : PendingToModify)
	{
		void* MapKeyPtr = MapHelper.GetKeyPtr(Index);
		void* PendingKeyPtr = FMemory::Malloc(KeyProperty->GetSize());
		if (PendingKeyPtr == nullptr)
		{
			continue;
		}

		ON_SCOPE_EXIT
		{
			KeyProperty->DestroyValue(PendingKeyPtr);
			FMemory::Free(PendingKeyPtr);
		};

		bool bKeyChanged = false;
		KeyProperty->CopyCompleteValue(PendingKeyPtr, MapKeyPtr);
		for (auto& Predicate : PredicatesForKey)
		{
			if (const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>())
			{
				bKeyChanged |= PredicatePtr->Execute(MapProperty->KeyProp, PendingKeyPtr, InParameters);
			}
		}

		if(bKeyChanged == true)
		{
			if (MapHelper.FindMapPairIndexFromHash(PendingKeyPtr) != INDEX_NONE)
			{
				FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Duplicate keys", "The modified key already exists in this map."));
				continue;
			}
			else
			{
				KeyProperty->CopyCompleteValue(MapKeyPtr, PendingKeyPtr);
			}
		}

		bool bValueChanged = false;
		void* MapValuePtr = MapHelper.GetValuePtr(Index);
		for (auto& Predicate : PredicatesForValue)
		{
			if (const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>())
			{
				bValueChanged |= PredicatePtr->Execute(MapProperty->ValueProp, MapValuePtr, InParameters);
			}
		}

		bExecutedSuccessfully = bKeyChanged || bValueChanged;
		MapHelper.Rehash();
	}

	return bExecutedSuccessfully;
}
#undef LOCTEXT_NAMESPACE
