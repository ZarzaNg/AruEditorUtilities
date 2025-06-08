#include "AssetPredicates/AruPredicate_Array.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AruPredicate_Array)
#define LOCTEXT_NAMESPACE "AruPredicate_Array"

bool FAruPredicate_AddArrayValue::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(InProperty);
	if (ArrayProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"AddArrayValue_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not an array."),
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
					"AddArrayValue_NoPredicates",
					"[{0}][{1}]Array:'{2}'. At least one predicate is required to complete the process."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(ArrayProperty->GetName())
			));
		return false;
	}

	FProperty* ElementProperty = ArrayProperty->Inner;
	void* PendingElementPtr = FMemory::Malloc(ElementProperty->GetSize());
	if (PendingElementPtr == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"AddArrayValue_MallocFailed",
					"[{0}][{1}]Map:'{2}'. Malloc memory for element failed."),
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
	for (auto& Predicate : Predicates)
	{
		const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>();
		if (PredicatePtr == nullptr)
		{
			continue;
		}

		bExecutedSuccessfully |= PredicatePtr->Execute(ElementProperty, PendingElementPtr, InParameters);
	}

	if(bExecutedSuccessfully == false)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"AddArrayValue_ExecutionFailure",
					"[{0}][{1}]Array:'{2}'. Predicate(s) executed failed."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(ArrayProperty->GetName())
			));
		return false;
	}

	FScriptArrayHelper ArrayHelper{ArrayProperty, InValue};
	int32 NewElementIndex = ArrayHelper.AddValue();
	if (!ArrayHelper.IsValidIndex(NewElementIndex))
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"AddArrayValue_AddFailed",
					"[{0}][{1}]Failed to add new element to array:'{2}'"),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error),
				FText::FromString(ArrayProperty->GetName())
			));
		return false;
	}

	void* NewElementPtr = ArrayHelper.GetRawPtr(NewElementIndex);
	ElementProperty->CopyCompleteValue(NewElementPtr, PendingElementPtr);

	FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
		FText::Format(
			LOCTEXT(
				"AddArray_Result.",
				"[{0}][{1}]Added element to array:'{2}'."),
			FText::FromString(GetCompactName()),
			FText::FromString(Aru::ProcessResult::Success),
			FText::FromString(ArrayProperty->GetName())
		));

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
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"RemoveFromArray_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not an array."),
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
				LOCTEXT(
					"RemoveFromArray_NoFilters",
					"[{0}][{1}]Array:'{2}'. At least one filter is required to complete the process."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(ArrayProperty->GetName())
			));
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

	FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
		FText::Format(
			LOCTEXT(
				"RemoveFromArray_Result.",
				"[{0}][{1}]Removed {2} element(s) from array:'{3}'."),
			FText::FromString(GetCompactName()),
			FText::FromString(PendingRemove.Num() > 0 ? Aru::ProcessResult::Success : Aru::ProcessResult::Failed),
			PendingRemove.Num(),
			FText::FromString(ArrayProperty->GetName()))
	);

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
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"ModifyArrayValue_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not an array."),
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
					"ModifyArrayValue_NoPredicates",
					"[{0}][{1}]Array:'{2}'. At least one predicate is required to complete the process."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(ArrayProperty->GetName())
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

			if (!Filter->IsConditionMet(ArrayProperty->Inner, ValuePtr, InParameters))
			{
				return false;
			}
		}
		return true;
	};

	FScriptArrayHelper ArrayHelper{ArrayProperty, InValue};
	int32 MatchedCount = 0;
	int32 ModifiedCount = 0;
	for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
	{
		void* ElementPtr = ArrayHelper.GetRawPtr(Index);
		if (!ShouldModify(ElementPtr))
		{
			continue;
		}
		else
		{
			MatchedCount++;
		}

		bool bElementModified = false;
		for (const TInstancedStruct<FAruPredicate>& PredicateStruct : Predicates)
		{
			const FAruPredicate* Predicate = PredicateStruct.GetPtr<FAruPredicate>();
			if (Predicate == nullptr)
			{
				continue;
			}

			bElementModified |= Predicate->Execute(ArrayProperty->Inner, ElementPtr, InParameters);
		}
		ModifiedCount += bElementModified ? 1 : 0;
	}

	FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
		FText::Format(
			LOCTEXT(
				"ModifyArrayValue_Result",
				"[{0}][{1}]Array:'{2}': {3} element(s) matched, {4} modified'."),
			FText::FromString(GetCompactName()),
			FText::FromString(ModifiedCount > 0 ? Aru::ProcessResult::Success : Aru::ProcessResult::Failed),
			FText::FromString(ArrayProperty->GetName()),
			MatchedCount,
			ModifiedCount
		));

	return ModifiedCount > 0;
}
#undef LOCTEXT_NAMESPACE