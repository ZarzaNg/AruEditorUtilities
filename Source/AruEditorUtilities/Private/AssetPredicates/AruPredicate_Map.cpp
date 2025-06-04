#include "AssetPredicates/AruPredicate_Map.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AruPredicate_Map)

#define LOCTEXT_NAMESPACE "AruPredicate_Map"

bool FAruPredicate_AddMapPair::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	const FMapProperty* MapProperty = CastField<FMapProperty>(InProperty);
	if (MapProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"AddToMap_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a map."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	if (PredicatesForKey.Num() == 0)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"AddToMap_NoPredicatesForKey",
					"[{0}][{1}]Map:'{2}'. At least one predicate is required to complete the process."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	FProperty* KeyProperty = MapProperty->KeyProp;
	FProperty* ValueProperty = MapProperty->ValueProp;
	if (KeyProperty == nullptr || ValueProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"AddToMap_ErrorSetup",
					"[{0}][{1}]Map:'{2}'. Can't get map inner property."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	void* PendingKeyPtr = FMemory::Malloc(KeyProperty->GetSize());
	if (PendingKeyPtr == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"AddToMap_MallocFailed",
					"[{0}][{1}]Map:'{2}'. Malloc memory for key failed."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error),
				FText::FromString(InProperty->GetName())
			));
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
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"AddToMap_ExecutionFailure",
					"[{0}][{1}]Map:'{2}'. Predicate(s) for key executed failed."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	FScriptMapHelper MapHelper{MapProperty, InValue};
	if (MapHelper.FindMapPairIndexFromHash(PendingKeyPtr) != INDEX_NONE)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"AddToMap_DuplicateKeys",
					"[{0}][{1}]The key pending to add already existed in this map:'{2}'."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	int32 NewElementIndex = MapHelper.AddDefaultValue_Invalid_NeedsRehash();
	if (!MapHelper.IsValidIndex(NewElementIndex))
	{
		MapHelper.RemoveAt(NewElementIndex);

		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"AddToMap_AddFailed",
					"[{0}][{1}]Failed to add new pair to map:'{2}'"),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error),
				FText::FromString(InProperty->GetName())
			));

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

		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"AddToMap_GetKeyFailed",
					"[{0}][{1}]Failed to get new key from map:'{2}'"),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error),
				FText::FromString(InProperty->GetName())
			));

		return false;
	}
	KeyProperty->CopyCompleteValue(NewKeyPtr, PendingKeyPtr);

	void* NewValuePtr = MapHelper.GetValuePtr(NewElementIndex);
	if (NewValuePtr == nullptr)
	{
		MapHelper.RemoveAt(NewElementIndex);

		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"AddToMap_GetValueFailed",
					"[{0}][{1}]Failed to get new value from map:'{2}'"),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error),
				FText::FromString(InProperty->GetName())
			));

		return false;
	}

	for (auto& Predicate : PredicatesForValue)
	{
		if (const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>())
		{
			PredicatePtr->Execute(ValueProperty, NewValuePtr, InParameters);
		}
	}

	FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
		FText::Format(
			LOCTEXT(
				"AddToMap_Result.",
				"[{0}][{1}]Added element to map:'{2}'."),
			FText::FromString(GetCompactName()),
			FText::FromString(Aru::ProcessResult::Success),
			FText::FromString(InProperty->GetName())
		));

	return bExecutedSuccessfully;
}

bool FAruPredicate_RemoveMapPair::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	const FMapProperty* MapProperty = CastField<FMapProperty>(InProperty);
	if (MapProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"RemoveFromMap_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a map."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	if (KeyFilters.Num() == 0 && ValueFilters.Num() == 0)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"RemoveFromMap_NoFilters",
					"[{0}][{1}]Map:'{2}'. At least one filter is required to complete the process."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	const FProperty* KeyProperty = MapProperty->KeyProp;
	const FProperty* ValueProperty = MapProperty->ValueProp;
	if (KeyProperty == nullptr || ValueProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"RemoveFromMap_ErrorSetup",
					"[{0}][{1}]Map:'{2}'. Can't get map inner property."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error),
				FText::FromString(InProperty->GetName())
			));
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

			if (!Filter->IsConditionMet(KeyProperty, KeyPtr, InParameters))
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

			if (!Filter->IsConditionMet(ValueProperty, ValuePtr, InParameters))
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

	FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
		FText::Format(
			LOCTEXT(
				"RemoveFromMap_Result.",
				"[{0}][{1}]Removed {2} element(s) from map:'{3}'."),
			FText::FromString(GetCompactName()),
			FText::FromString(PendingRemove.Num() > 0 ? Aru::ProcessResult::Success : Aru::ProcessResult::Failed),
			PendingRemove.Num(),
			FText::FromString(InProperty->GetName()))
	);

	return PendingRemove.Num() > 0;
}

bool FAruPredicate_ModifyMapPair::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	const FMapProperty* MapProperty = CastField<FMapProperty>(InProperty);
	if (MapProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"ModifyMapValue_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a map."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	if (KeyFilters.Num() == 0 && ValueFilters.Num() == 0)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"ModifyMapValue_NoFilters",
					"[{0}][{1}]Map:'{2}'. At least one filter is required to complete the process."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	const FProperty* KeyProperty = MapProperty->KeyProp;
	const FProperty* ValueProperty = MapProperty->ValueProp;
	if (KeyProperty == nullptr || ValueProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"ModifyMapValue_ErrorSetup",
					"[{0}][{1}]Map:'{2}'. Can't get map inner property."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error),
				FText::FromString(InProperty->GetName())
			));
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

	int32 ModifiedCount = 0;
	for (int32& Index : PendingToModify)
	{
		void* MapKeyPtr = MapHelper.GetKeyPtr(Index);
		void* PendingKeyPtr = FMemory::Malloc(KeyProperty->GetSize());
		if (PendingKeyPtr == nullptr)
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"ModifyMapValue_MallocFailed",
					"[{0}][{1}]Map:'{2}'. Malloc memory for key failed."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error),
				FText::FromString(InProperty->GetName())
			));
			
			return false;
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

		if (bKeyChanged == true)
		{
			if (MapHelper.FindMapPairIndexFromHash(PendingKeyPtr) != INDEX_NONE)
			{
				FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
					FText::Format(
						LOCTEXT(
							"ModifyMapValue_DuplicateKeys",
							"[{0}][{1}]The key pending to set already existed in this map:'{2}'."),
						FText::FromString(GetCompactName()),
						FText::FromString(Aru::ProcessResult::Failed),
						FText::FromString(InProperty->GetName())
					));
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

		ModifiedCount += bKeyChanged || bValueChanged ? 1 : 0;
		MapHelper.Rehash();
	}

	FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
		FText::Format(
			LOCTEXT(
				"ModifyMapValue_Result",
				"[{0}][{1}]Map:'{2}': {3} element(s) matched, {4} modified'."),
			FText::FromString(GetCompactName()),
			FText::FromString(ModifiedCount > 0 ? Aru::ProcessResult::Success : Aru::ProcessResult::Failed),
			FText::FromString(InProperty->GetName()),
			PendingToModify.Num(),
			ModifiedCount
		));

	return ModifiedCount > 0;
}
#undef LOCTEXT_NAMESPACE
