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
					"PropertyTypeMismatch",
					"Property:'{0}' is not a set."),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	if (Predicates.Num() == 0)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT("Execution failed", "{0}: Lack of predicate configuration."),
				FText::FromString(GetNameSafe(StaticStruct()))));
		return false;
	}

	FProperty* ElementProperty = SetProperty->ElementProp;
	if (ElementProperty == nullptr)
	{
		return false;
	}

	void* PendingElementPtr = FMemory::Malloc(ElementProperty->GetSize());
	if (PendingElementPtr == nullptr)
	{
		return false;
	}

	ON_SCOPE_EXIT
	{
		ElementProperty->DestroyValue(PendingElementPtr);
		FMemory::Free(PendingElementPtr);
	};

	bool bExecutedSuccessfully = false;
	ElementProperty->InitializeValue(PendingElementPtr);
	for (auto& Predicate : Predicates)
	{
		const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>();
		if (PredicatePtr == nullptr)
		{
			continue;
		}

		bExecutedSuccessfully |= PredicatePtr->Execute(ElementProperty, PendingElementPtr, InParameters);
	}

	if (bExecutedSuccessfully == false)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT("Execution failed", "{0}: Execution failed."),
				FText::FromString(GetNameSafe(StaticStruct()))));
		return false;
	}

	FScriptSetHelper SetHelper(SetProperty, InValue);
	if (SetHelper.FindElementIndex(PendingElementPtr) != INDEX_NONE)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT("Execution failed", "{0}: The value pending to add already exists in this set."),
				FText::FromString(GetNameSafe(StaticStruct()))));
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
					"PropertyTypeMismatch",
					"Property:'{0}' is not a set."),
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
					"PropertyTypeMismatch",
					"Property:'{0}' is not a set."),
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
				LOCTEXT("Execution failed", "{0}: Lack of filter configuration."),
				FText::FromString(GetNameSafe(StaticStruct()))));
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

	bool bExecutedSuccessfully = false;
	FScriptSetHelper SetHelper{SetProperty, InValue};
	for (int32 Index = 0; Index < SetHelper.Num(); ++Index)
	{
		void* ElementPtr = SetHelper.GetElementPtr(Index);
		if (!ShouldModify(ElementPtr))
		{
			continue;
		}

		void* PendingElementPtr = FMemory::Malloc(ElementProperty->GetSize());
		if (PendingElementPtr == nullptr)
		{
			return false;
		}

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
				LOCTEXT("Execution failed", "{0}: The value pending to add already exists in this set."),
				FText::FromString(GetNameSafe(StaticStruct()))));
			continue;
		}

		ElementProperty->CopyCompleteValue(ElementPtr, PendingElementPtr);
		SetHelper.Rehash();

		bExecutedSuccessfully |= bValueChanged;
	}

	return bExecutedSuccessfully;
}

#undef LOCTEXT_NAMESPACE
