#include "AssetPredicates/AruPredicate_GameplayTag.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AruPredicate_GameplayTag)

#define LOCTEXT_NAMESPACE "AruPredicate_GameplayTag"

bool FAruPredicate_SetGameplayTag::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"InPropertyNull",
					"[{0}][{1}]In Property is NULL."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error)
			));
		return false;
	}

	if (InValue == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"InValueNull",
					"[{0}][{1}]In Value is NULL."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error)
			));
		return false;
	}

	const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty);
	if (StructProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetInstancedStructProperty_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a struct property."),
						FText::FromString(GetCompactName()),
						FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	const UScriptStruct* SourceStructType = StructProperty->Struct;
	if (SourceStructType == nullptr || SourceStructType != FGameplayTag::StaticStruct())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetInstancedStructProperty_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a gameplay tag property."),
						FText::FromString(GetCompactName()),
						FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	FGameplayTag* GameplayTagPtr = static_cast<FGameplayTag*>(InValue);
	if (GameplayTagPtr == nullptr)
	{
		return false;
	}

	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStructProperty>(InParameters, FGameplayTag::StaticStruct());
	if (!OptionalValue.IsSet())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"PropertySetter_NewValueNoFound",
					"[{0}][{1}]Property:'{2}': can't find new value by source type:'{3}'."),
						FText::FromString(GetCompactName()),
						FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName()),
				FText::FromString(StaticEnum<EAruValueSource>()->GetValueAsString(ValueSource))
			));
		return false;
	}
	
	const void* PendingValue = OptionalValue.GetValue();
	if (PendingValue == nullptr)
	{
		return false;
	}

	StructProperty->CopyCompleteValue(InValue, PendingValue);
	
	FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
		FText::Format(
			LOCTEXT(
				"SetGameplayTag_Result",
				"[{0}][{1}]Property:'{2}': operation succeeded."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Success)
		));
	
	return true;
}

bool FAruPredicate_SetGameplayTagContainer::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"InPropertyNull",
					"[{0}][{1}]In Property is NULL."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error)
			));
		return false;
	}

	if (InValue == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"InValueNull",
					"[{0}][{1}]In Value is NULL."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error)
			));
		return false;
	}

	const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty);
	if (StructProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetInstancedStructProperty_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a struct property."),
						FText::FromString(GetCompactName()),
						FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	const UScriptStruct* SourceStructType = StructProperty->Struct;
	if (SourceStructType == nullptr || SourceStructType != FGameplayTagContainer::StaticStruct())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetInstancedStructProperty_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a gameplay tag container property."),
						FText::FromString(GetCompactName()),
						FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	FGameplayTagContainer* GameplayTagPtr = static_cast<FGameplayTagContainer*>(InValue);
	if (GameplayTagPtr == nullptr)
	{
		return false;
	}

	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStructProperty>(InParameters, FGameplayTagContainer::StaticStruct());
	if (!OptionalValue.IsSet())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"PropertySetter_NewValueNoFound",
					"[{0}][{1}]Property:'{2}': can't find new value by source type:'{3}'."),
						FText::FromString(GetCompactName()),
						FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName()),
				FText::FromString(StaticEnum<EAruValueSource>()->GetValueAsString(ValueSource))
			));
		return false;
	}
	
	const void* PendingValue = OptionalValue.GetValue();
	if (PendingValue == nullptr)
	{
		return false;
	}
	
	StructProperty->CopyCompleteValue(InValue, PendingValue);

	FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
		FText::Format(
			LOCTEXT(
				"SetGameplayTagContainer_Result",
				"[{0}][{1}]Property:'{2}': operation succeeded."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Success)
		));
	
	return true;
}
#undef LOCTEXT_NAMESPACE