#include "AssetPredicates/AruPredicate_Set.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AruPredicate_Set)

#define LOCTEXT_NAMESPACE "AruPredicate_Set"

bool FAruPredicate_AddSetElement::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	const FSetProperty* SetProperty = CastField<FSetProperty>(InProperty);
	if (SetProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"AddToSet_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a set."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	if (Predicates.Num() == 0)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"AddToSet_NoPredicatesForKey",
					"[{0}][{1}]Set:'{2}'. At least one predicate is required to complete the process."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	FProperty* ElementProperty = SetProperty->ElementProp;
	if (ElementProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"AddToSet_GetElementPropertyFailed",
					"[{0}][{1}]Set:'{2}'. Can't get element property."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error),
				FText::FromString(InProperty->GetName())
			));
		
		return false;
	}

	void* PendingElementPtr = FMemory::Malloc(ElementProperty->GetSize());
	if (PendingElementPtr == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"AddToSet__MallocFailed",
					"[{0}][{1}]Set:'{2}'. Malloc memory for element failed."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error),
				FText::FromString(InProperty->GetName())
			));
		
		return false;
	}
	ElementProperty->InitializeValue(PendingElementPtr);

	ON_SCOPE_EXIT
	{
		ElementProperty->DestroyValue(PendingElementPtr);
		FMemory::Free(PendingElementPtr);
	};

	bool bExecutedSuccessfully = false;
	ElementProperty->InitializeValue(PendingElementPtr);
	for (auto& Predicate : Predicates)
	{
		if (const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>())
		{
			bExecutedSuccessfully |= PredicatePtr->Execute(ElementProperty, PendingElementPtr, InParameters);
		}
	}

	if (bExecutedSuccessfully == false)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"AddToSet_ExecutionFailure",
					"[{0}][{1}]Set:'{2}'. Predicate(s) for element executed failed."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	FScriptSetHelper SetHelper(SetProperty, InValue);
	if (SetHelper.FindElementIndex(PendingElementPtr) != INDEX_NONE)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"AddToSet_DuplicateElements",
					"[{0}][{1}]The element pending to add already existed in this set:'{2}'."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	const int32 NewElementIndex = SetHelper.AddDefaultValue_Invalid_NeedsRehash();
	ON_SCOPE_EXIT
	{
		SetHelper.Rehash();
	};

	void* NewElementPtr = SetHelper.GetElementPtr(NewElementIndex);
	if (NewElementPtr == nullptr)
	{
		SetHelper.RemoveAt(NewElementIndex);
		return false;
	}

	ElementProperty->CopyCompleteValue(NewElementPtr, PendingElementPtr);
	return true;
}

bool FAruPredicate_RemoveSetValue::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	const FSetProperty* SetProperty = CastField<FSetProperty>(InProperty);
	if (SetProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"AddToSet_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a set."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	if (Filters.Num() == 0)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT("Execution failed", "{0}: Lack of filter configuration."),
				FText::FromString(GetNameSafe(StaticStruct()))));
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

			if (!Filter->IsConditionMet(SetProperty->ElementProp, ValuePtr, InParameters))
			{
				return false;
			}
		}
		return true;
	};

	TArray<int32> PendingRemove;
	FScriptSetHelper SetHelper{SetProperty, InValue};
	for (int32 Index = 0; Index < SetHelper.Num(); ++Index)
	{
		if (ShouldRemove(SetHelper.GetElementPtr(Index)))
		{
			PendingRemove.Add(Index);
		}
	}

	for (int32& Index : PendingRemove)
	{
		SetHelper.RemoveAt(Index);
	}

	FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
		FText::Format(
			LOCTEXT(
				"RemoveFromSet_Result.",
				"[{0}][{1}]Removed {2} element(s) from set:'{3}'."),
			FText::FromString(GetCompactName()),
			FText::FromString(PendingRemove.Num() > 0 ? Aru::ProcessResult::Success : Aru::ProcessResult::Failed),
			PendingRemove.Num(),
			FText::FromString(InProperty->GetName()))
	);

	return PendingRemove.Num() > 0;
}

bool FAruPredicate_ModifySetValue::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	const FSetProperty* SetProperty = CastField<FSetProperty>(InProperty);
	if (SetProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"AddToSet_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a set."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		
		return false;
	}

	const FProperty* ElementProperty = SetProperty->ElementProp;
	if (ElementProperty == nullptr)
	{
		return false;
	}

	if (Filters.Num() == 0)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"ModifySetValue_NoFilters",
					"[{0}][{1}]Set:'{2}'. At least one filter is required to complete the process."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		
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

			if (!Filter->IsConditionMet(SetProperty->ElementProp, ValuePtr, InParameters))
			{
				return false;
			}
		}
		return true;
	};

	int32 MatchedCount = 0;
	int32 ModifiedCount = 0;
	FScriptSetHelper SetHelper{SetProperty, InValue};
	for (int32 Index = 0; Index < SetHelper.Num(); ++Index)
	{
		void* ElementPtr = SetHelper.GetElementPtr(Index);
		if (!ShouldModify(ElementPtr))
		{
			continue;
		}
		else
		{
			MatchedCount++;
		}

		void* PendingElementPtr = FMemory::Malloc(ElementProperty->GetSize());
		if (PendingElementPtr == nullptr)
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"ModifySetValue_MallocFailed",
					"[{0}][{1}]Set:'{2}'. Malloc memory for element failed."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error),
				FText::FromString(InProperty->GetName())
			));
			
			return false;
		}
		ElementProperty->InitializeValue(PendingElementPtr);

		ON_SCOPE_EXIT
		{
			ElementProperty->DestroyValue(PendingElementPtr);
			FMemory::Free(PendingElementPtr);
		};

		bool bValueChanged = false;
		ElementProperty->CopyCompleteValue(PendingElementPtr, ElementPtr);
		for (const TInstancedStruct<FAruPredicate>& PredicateStruct : Predicates)
		{
			const FAruPredicate* Predicate = PredicateStruct.GetPtr<FAruPredicate>();
			if (Predicate == nullptr)
			{
				continue;
			}

			bValueChanged |= Predicate->Execute(SetProperty->ElementProp, PendingElementPtr, InParameters);
		}

		if(bValueChanged == false)
		{
			continue;
		}

		if (SetHelper.FindElementIndex(PendingElementPtr) != INDEX_NONE)
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
					FText::Format(
						LOCTEXT(
							"ModifySetValue_DuplicateElements",
							"[{0}][{1}]The element pending to set already existed in this map:'{2}'."),
						FText::FromString(GetCompactName()),
						FText::FromString(Aru::ProcessResult::Failed),
						FText::FromString(InProperty->GetName())
					));
			
			continue;
		}

		ElementProperty->CopyCompleteValue(ElementPtr, PendingElementPtr);
		SetHelper.Rehash();

		ModifiedCount += bValueChanged? 1 : 0;
	}

	FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
		FText::Format(
			LOCTEXT(
				"ModifySetValue_Result",
				"[{0}][{1}]Array:'{2}': {3} element(s) matched, {4} modified'."),
			FText::FromString(GetCompactName()),
			FText::FromString(ModifiedCount > 0 ? Aru::ProcessResult::Success : Aru::ProcessResult::Failed),
			FText::FromString(InProperty->GetName()),
			MatchedCount,
			ModifiedCount
		));

	return ModifiedCount > 0;
}

#undef LOCTEXT_NAMESPACE
