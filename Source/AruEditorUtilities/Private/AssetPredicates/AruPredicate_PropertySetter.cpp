#include "AssetPredicates/AruPredicate_PropertySetter.h"
#include "AruFunctionLibrary.h"
#include "UObject/PropertyAccessUtil.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AruPredicate_PropertySetter)

#define LOCTEXT_NAMESPACE "AruPredicate_PropertySetter"

bool FAruPredicate_PropertySetter::IsCompatibleType(
	const FProperty* TargetProperty,
	const void* TargetValue,
	const UStruct* SourceType)
{
	if (TargetProperty == nullptr || SourceType == nullptr)
	{
		return true;
	}

	if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(TargetProperty))
	{
		return IsCompatibleObjectType(ObjectProperty, TargetValue, SourceType);
	}

	if (const FStructProperty* StructProperty = CastField<FStructProperty>(TargetProperty))
	{
		return IsCompatibleStructType(StructProperty, TargetValue, SourceType);
	}

	return false;
}

bool FAruPredicate_PropertySetter::IsCompatibleObjectType(
	const FObjectPropertyBase* TargetProperty,
	const void* TargetValue,
	const UStruct* SourceType)
{
	const UObject* Object = TargetProperty->GetObjectPropertyValue(TargetValue);
	if (Object == nullptr)
	{
		// We might want to clear the property value.
		// So when we got nullptr, we consider it as matched.
		return true;
	}

	const UClass* ObjectClass = Object->GetClass();
	if (ObjectClass == nullptr)
	{
		return false;
	}

	return ObjectClass->IsChildOf(SourceType);
}

bool FAruPredicate_PropertySetter::IsCompatibleStructType(
	const FStructProperty* TargetProperty,
	const void* TargetValue,
	const UStruct* SourceType)
{
	const UScriptStruct* ScriptStruct = TargetProperty->Struct;
	if (ScriptStruct == nullptr)
	{
		return false;
	}

	if (SourceType == FInstancedStruct::StaticStruct())
	{
		return ScriptStruct == FInstancedStruct::StaticStruct();
	}

	const FInstancedStruct* InstancedStructPtr = static_cast<const FInstancedStruct*>(TargetValue);
	if (InstancedStructPtr == nullptr)
	{
		return false;
	}

	const UScriptStruct* InnerStruct = InstancedStructPtr->GetScriptStruct();
	if (InnerStruct == nullptr)
	{
		return false;
	}

	return InnerStruct->IsChildOf(SourceType);
}

TOptional<const void*> FAruPredicate_PropertySetter::GetValueFromStructProperty(
	const FFieldClass* SourceProperty,
	const UStruct* SourceType) const
{
	static FName ValueName{"NewValue"};
	const FProperty* TargetProperty = PropertyAccessUtil::FindPropertyByName(ValueName, GetScriptedStruct());
	if (!ensureMsgf(TargetProperty != nullptr, TEXT("Can't find NewValue property in derived struct.")))
	{
		return {};
	}

	if (!TargetProperty->IsA(SourceProperty))
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"PropertySetter_PropertyTypeMismatch",
					"[{0}][{1}]Mismatch between target:'{2}' and source:'{3}' property types."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(TargetProperty->GetName()),
				FText::FromString(SourceProperty->GetName())
			));
		return {};
	}

	const void* PropertyValue = TargetProperty->ContainerPtrToValuePtr<void>(this);
	if (!IsCompatibleType(TargetProperty, PropertyValue, SourceType))
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"PropertySetter_PropertyValueMismatch",
					"[{0}][{1}]Mismatch between target:'{2}' and source:'{3}' value types."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(TargetProperty->GetName()),
				FText::FromString(SourceProperty->GetName())
			));
		return {};
	}

	return TOptional<const void*>{PropertyValue};
}

TOptional<const void*> FAruPredicate_PropertySetter::GetValueFromObjectAsset(
	const FFieldClass* SourceProperty,
	const FInstancedPropertyBag& InParameters,
	const UStruct* SourceType) const
{
	if (PathToProperty.IsEmpty())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"PropertySetter_PathEmpty",
					"[{0}][{1}]PathToProperty Empty, please check your config."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed)
			));
		return {};
	}

	if (Object == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"PropertySetter_ObjectNull",
					"[{0}][{1}]Object is NULL, please check your config."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed)
			));
		return {};
	}

	const UClass* NativeClass = Object.GetClass();
	const UObject* NativeObject = Object;
	if (UBlueprint* BlueprintObject = Cast<UBlueprint>(Object))
	{
		NativeClass = BlueprintObject->GeneratedClass;
		NativeObject = NativeClass->GetDefaultObject();
	}

	TArray<FString> PropertyChain;
	PathToProperty.ParseIntoArray(PropertyChain, TEXT("."), true);
	for (auto& Element : PropertyChain)
	{
		Element = UAruFunctionLibrary::ResolveParameterizedString(InParameters, Element);
	}
	const FString&& ResolvedPath = FString::Join(PropertyChain, TEXT("."));

	auto&& PropertyContext = UAruFunctionLibrary::FindPropertyByPath(NativeClass, NativeObject, ResolvedPath);
	if (!PropertyContext.IsValid())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"PropertySetter_NoPropertyFound",
					"[{0}][{1}]Can't find property by path: '{2}' in object: '{3}'."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(ResolvedPath),
				FText::FromString(Object.GetName())
			));
		return {};
	}

	if (!PropertyContext.PropertyPtr->IsA(SourceProperty))
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"PropertySetter_PropertyTypeMismatch",
					"[{0}][{1}]Mismatch between target:'{2}' and source:'{3}' property types."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(PropertyContext.PropertyPtr->GetName()),
				FText::FromString(SourceProperty->GetName())
			));

		return {};
	}

	if (!IsCompatibleType(PropertyContext.PropertyPtr, PropertyContext.ValuePtr.GetValue(), SourceType))
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"PropertySetter_PropertyValueMismatch",
					"[{0}][{1}]Mismatch between target:'{2}' and source:'{3}' value types."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(PropertyContext.PropertyPtr->GetName()),
				FText::FromString(SourceProperty->GetName())
			));

		return {};
	}

	return TOptional<const void*>{PropertyContext.ValuePtr.GetValue()};
}

TOptional<const void*> FAruPredicate_PropertySetter::GetValueFromDataTable(
	const FFieldClass* SourceProperty,
	const FInstancedPropertyBag& InParameters,
	const UStruct* SourceType) const
{
	if (PathToProperty.IsEmpty())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"PropertySetter_PathEmpty",
					"[{0}][{1}]PathToProperty Empty, please check your config."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed)
			));
		return {};
	}

	if (DataTable == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"PropertySetter_DataTableNull",
					"[{0}][{1}]DataTable is NULL, please check your config."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed)
			));
	}

	if (RowName.IsEmpty())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"PropertySetter_RowNameEmpty",
					"[{0}][{1}]RowName Empty, please check your config."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed)
			));
	}

	const FString&& ResolvedRowName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, RowName);
	uint8* const* RowStructPtr = DataTable->GetRowMap().Find(FName{ResolvedRowName});
	if (RowStructPtr == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT("PropertySetter_FindRowFailed", "[{0}][{1}]Can't find row: '{2}' in DataTable: '{3}'."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(ResolvedRowName),
				FText::FromName(DataTable.GetFName())));
		return {};
	}

	const uint8* RowStruct = *RowStructPtr;
	if (RowStruct == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT("PropertySetter_FindRowFailed", "[{0}][{1}]Can't find row: '{2}' in DataTable: '{3}'."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(ResolvedRowName),
				FText::FromName(DataTable.GetFName())));
		return {};
	}

	TArray<FString> PropertyChain;
	PathToProperty.ParseIntoArray(PropertyChain, TEXT("."), true);
	for (auto& Element : PropertyChain)
	{
		Element = UAruFunctionLibrary::ResolveParameterizedString(InParameters, Element);
	}
	const FString&& ResolvedPath = FString::Join(PropertyChain, TEXT("."));

	auto&& PropertyContext = UAruFunctionLibrary::FindPropertyByPath(DataTable->RowStruct, RowStruct, ResolvedPath);
	if (!PropertyContext.IsValid())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT("PropertySetter_NoPropertyFoundInStruct", "[{0}][{1}]Can't find property by path: '{2}' in struct: '{3}'."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(PathToProperty),
				FText::FromName(DataTable->RowStruct.GetFName())));
		return {};
	}

	if (!PropertyContext.PropertyPtr->IsA(SourceProperty))
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"PropertySetter_PropertyTypeMismatch",
					"[{0}][{1}]Mismatch between target:'{2}' and source:'{3}' property types."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(PropertyContext.PropertyPtr->GetName()),
				FText::FromString(SourceProperty->GetName())
			));
		return {};
	}

	if (!IsCompatibleType(PropertyContext.PropertyPtr, PropertyContext.ValuePtr.GetValue(), SourceType))
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"PropertySetter_PropertyValueMismatch",
					"[{0}][{1}]Mismatch between target:'{2}' and source:'{3}' value types."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(PropertyContext.PropertyPtr->GetName()),
				FText::FromString(SourceProperty->GetName())
			));
		return {};
	}

	return TOptional<const void*>{PropertyContext.ValuePtr.GetValue()};
}

bool FAruPredicate_SetBoolValue::Execute(
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

	const FBoolProperty* BoolProperty = CastField<FBoolProperty>(InProperty);
	if (BoolProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetBoolProperty_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a bool property."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	bool Result = false;
	const bool PreviousValue = BoolProperty->GetPropertyValue(InValue);
	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<bool, EPropertyBagResult> ParameterValue = InParameters.GetValueBool(FName{ResolvedParameterName});
		if (!ParameterValue.HasValue())
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
				FText::Format(
					LOCTEXT(
						"NoPropertyFoundInParameter",
						"[{0}][{1}]Property:'{2}'. Can't find '{3}' in parameters."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Failed),
					FText::FromString(InProperty->GetName()),
					FText::FromString(ResolvedParameterName)
				));
			return false;
		}
		BoolProperty->SetPropertyValue(InValue, ParameterValue.GetValue());
		Result = true;
	}
	else
	{
		Result = SetPropertyValue<FBoolProperty>(InProperty, InValue, InParameters);
	}

	if (Result == true)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
			FText::Format(
				LOCTEXT(
					"SetBoolProperty_Success",
					"[{0}][{1}]Property:'{2}' previous value:{3}, new value:{4}"),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Success),
				FText::FromString(InProperty->GetName()),
				PreviousValue ? LOCTEXT("True", "True") : LOCTEXT("False", "False"),
				BoolProperty->GetPropertyValue(InValue) ? LOCTEXT("True", "True") : LOCTEXT("False", "False")
			));
	}
	else
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetProperty_Failed",
					"[{0}][{1}]Property:'{2}' operation failure."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
	}

	return Result;
}

bool FAruPredicate_SetFloatValue::Execute(
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

	const FNumericProperty* NumericProperty = CastField<FNumericProperty>(InProperty);
	if (NumericProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetFloatProperty_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a bool property."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	if (!NumericProperty->IsFloatingPoint())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetFloatProperty_NumericTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a float property."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	bool Result = false;
	const float PreviousValue = NumericProperty->GetFloatingPointPropertyValue(InValue);
	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<double, EPropertyBagResult> ParameterValue = InParameters.GetValueDouble(FName{ResolvedParameterName});
		if (!ParameterValue.HasValue())
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
				FText::Format(
					LOCTEXT(
						"NoPropertyFoundInParameter",
						"[{0}][{1}]Property:'{2}'. Can't find '{3}' in parameters."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Failed),
					FText::FromString(InProperty->GetName()),
					FText::FromString(ResolvedParameterName)
				));
			return false;
		}
		NumericProperty->SetFloatingPointPropertyValue(InValue, ParameterValue.GetValue());
		Result = true;
	}
	else
	{
		Result = SetPropertyValue<FNumericProperty>(InProperty, InValue, InParameters);
	}
	
	if (Result == true)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
			FText::Format(
				LOCTEXT(
					"SetFloatProperty_Success",
					"[{0}][{1}]Property:'{2}' previous value:{3}, new value:{4}"),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Success),
				FText::FromString(InProperty->GetName()),
				PreviousValue,
				NumericProperty->GetFloatingPointPropertyValue(InValue)
			));
	}
	else
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetProperty_Failed",
					"[{0}][{1}]Property:'{2}' operation failure."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
	}

	return Result;
}

bool FAruPredicate_SetIntegerValue::Execute(
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

	const FNumericProperty* NumericProperty = CastField<FNumericProperty>(InProperty);
	if (NumericProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetIntegerProperty_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a integer property."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	if (!NumericProperty->IsInteger())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetIntegerProperty_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a integer property."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	bool Result = false;
	const float PreviousValue = NumericProperty->GetSignedIntPropertyValue(InValue);
	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<int64, EPropertyBagResult> ParameterValue = InParameters.GetValueInt64(FName{ResolvedParameterName});
		if (!ParameterValue.HasValue())
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
				FText::Format(
					LOCTEXT(
						"NoPropertyFoundInParameter",
						"[{0}][{1}]Property:'{2}'. Can't find '{3}' in parameters."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Failed),
					FText::FromString(InProperty->GetName()),
					FText::FromString(ResolvedParameterName)
				));
			return false;
		}
		NumericProperty->SetIntPropertyValue(InValue, ParameterValue.GetValue());
		Result = true;
	}
	else
	{
		Result = SetPropertyValue<FNumericProperty>(InProperty, InValue, InParameters);
	}

	if (Result == true)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
			FText::Format(
				LOCTEXT(
					"SetIntegerProperty_Success",
					"[{0}][{1}]Property:'{2}' previous value:{3}, new value:{4}"),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Success),
				FText::FromString(InProperty->GetName()),
				PreviousValue,
				NumericProperty->GetSignedIntPropertyValue(InValue)
			));
	}
	else
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetProperty_Failed",
					"[{0}][{1}]Property:'{2}' operation failure."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
	}

	return Result;
}

bool FAruPredicate_SetStringValue::Execute(
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

	const FStrProperty* StrProperty = CastField<FStrProperty>(InProperty);
	if (StrProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetStrProperty_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a string property."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	bool Result = false;
	const FString PreviousValue = StrProperty->GetPropertyValue(InValue);
	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<FString, EPropertyBagResult> ParameterValue = InParameters.GetValueString(FName{ResolvedParameterName});
		if (!ParameterValue.HasValue())
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
				FText::Format(
					LOCTEXT(
						"NoPropertyFoundInParameter",
						"[{0}][{1}]Property:'{2}'. Can't find '{3}' in parameters."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Failed),
					FText::FromString(InProperty->GetName()),
					FText::FromString(ResolvedParameterName)
				));
			return false;
		}
		StrProperty->SetPropertyValue(InValue, ParameterValue.GetValue());
		Result = true;
	}
	else
	{
		Result = SetPropertyValue<FStrProperty>(InProperty, InValue, InParameters);
	}

	if (Result == true)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
			FText::Format(
				LOCTEXT(
					"SetStrProperty_Success",
					"[{0}][{1}]Property:'{2}' previous value:{3}, new value:{4}"),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Success),
				FText::FromString(InProperty->GetName()),
				FText::FromString(PreviousValue),
				FText::FromString(StrProperty->GetPropertyValue(InValue))
			));
	}
	else
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetProperty_Failed",
					"[{0}][{1}]Property:'{2}' operation failure."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
	}
	return Result;
}

bool FAruPredicate_SetTextValue::Execute(
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

	const FTextProperty* TextProperty = CastField<FTextProperty>(InProperty);
	if (TextProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetTextProperty_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{0}' is not a text property."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	bool Result = false;
	const FText PreviousValue = TextProperty->GetPropertyValue(InValue);
	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<FText, EPropertyBagResult> ParameterValue = InParameters.GetValueText(FName{ResolvedParameterName});
		if (!ParameterValue.HasValue())
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
				FText::Format(
					LOCTEXT(
						"NoPropertyFoundInParameter",
						"[{0}][{1}]Property:'{2}'. Can't find '{3}' in parameters."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Failed),
					FText::FromString(InProperty->GetName()),
					FText::FromString(ResolvedParameterName)
				));
			return false;
		}
		TextProperty->SetPropertyValue(InValue, ParameterValue.GetValue());
		Result = true;
	}
	else
	{
		TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStrProperty>(InParameters);
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

		const FString* StringValue = static_cast<const FString*>(PendingValue);
		if (StringValue == nullptr)
		{
			return false;
		}

		FText PendingTextValue = FText::FromString(*StringValue);
		TextProperty->CopyCompleteValue(InValue, &PendingTextValue);
		Result = true;
	}

	if (Result == true)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
			FText::Format(
				LOCTEXT(
					"SetTextProperty_Success",
					"[{0}][{1}]Property:'{2}' previous value:{3}, new value:{4}"),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Success),
				FText::FromString(InProperty->GetName()),
				PreviousValue,
				TextProperty->GetPropertyValue(InValue)
			));
	}
	else
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetProperty_Failed",
					"[{0}][{1}]Property:'{2}' operation failure."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
	}
	
	return Result;
}

bool FAruPredicate_SetNameValue::Execute(
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

	const FNameProperty* NameProperty = CastField<FNameProperty>(InProperty);
	if (NameProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"Name_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not an FName property."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}
	
	bool Result = false;
	const FName PreviousValue = NameProperty->GetPropertyValue(InValue);
	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<FName, EPropertyBagResult> ParameterValue = InParameters.GetValueName(FName{ResolvedParameterName});
		if (!ParameterValue.HasValue())
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
				FText::Format(
					LOCTEXT(
						"NoPropertyFoundInParameter",
						"[{0}][{1}]Property:'{2}'. Can't find '{3}' in parameters."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Failed),
					FText::FromString(InProperty->GetName()),
					FText::FromString(ResolvedParameterName)
				));
			return false;
		}
		NameProperty->SetPropertyValue(InValue, ParameterValue.GetValue());
		Result = true;
	}
	else
	{
		Result = SetPropertyValue<FNameProperty>(InProperty, InValue, InParameters);
	}
	
	if (Result == true)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
			FText::Format(
				LOCTEXT(
					"SetNameProperty_Success",
					"[{0}][{1}]Property:'{2}' previous value:{3}, new value:{4}"),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Success),
				FText::FromString(InProperty->GetName()),
				FText::FromName(PreviousValue),
				FText::FromName(NameProperty->GetPropertyValue(InValue))
			));
	}
	else
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetProperty_Failed",
					"[{0}][{1}]Property:'{2}' operation failure."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
	}

	return Result;
}

bool FAruPredicate_SetEnumValue::Execute(
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

	const FEnumProperty* EnumProperty = CastField<FEnumProperty>(InProperty);
	if (EnumProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetEnumProperty_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a enum property."),
						FText::FromString(GetCompactName()),
						FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	const UEnum* EnumType = EnumProperty->GetEnum();
	if (EnumType == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetEnumProperty_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' enum type is NULL."),
						FText::FromString(GetCompactName()),
						FText::FromString(Aru::ProcessResult::Failed),
					FText::FromString(InProperty->GetName())
			));
		return false;
	}

	const FNumericProperty* UnderlyingProperty = EnumProperty->GetUnderlyingProperty();
	if (UnderlyingProperty == nullptr)
	{
		return false;
	}

	bool Result = false;
	const int64 PreviousValue = UnderlyingProperty->GetSignedIntPropertyValue(InValue);
	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<uint8, EPropertyBagResult> ParameterValue = InParameters.GetValueEnum(FName{ResolvedParameterName}, EnumType);
		if (!ParameterValue.HasValue())
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
				FText::Format(
					LOCTEXT(
						"NoPropertyFoundInParameter",
						"[{0}][{1}]Property:'{2}'. Can't find '{3}' in parameters."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Failed),
					FText::FromString(InProperty->GetName()),
					FText::FromString(ResolvedParameterName)
				));
			return false;
		}
		UnderlyingProperty->SetIntPropertyValue(InValue, static_cast<int64>(ParameterValue.GetValue()));
		Result = true;
	}
	else
	{
		TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStrProperty>(InParameters);
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

		const FString* StringValue = static_cast<const FString*>(PendingValue);
		if (StringValue == nullptr)
		{
			return false;
		}

		const int64 PendingEnumValue = EnumType->GetValueByNameString(*StringValue);
		if (PendingEnumValue == INDEX_NONE)
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
			FText::Format(
				LOCTEXT(
					"SetEnumProperty_NoEnumFound",
					"[{0}][{1}]Property:'{2}', can't find valid enum value by '{3}'."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Success),
				FText::FromString(InProperty->GetName()),
				FText::FromString(*StringValue)
			));
			return false;
		}

		UnderlyingProperty->SetIntPropertyValue(InValue, PendingEnumValue);
		Result = true;
	}
	
	if (Result == true)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
			FText::Format(
				LOCTEXT(
					"SetEnumProperty_Success",
					"[{0}][{1}]Property:'{2}' previous value:{3}, new value:{4}"),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Success),
				FText::FromString(InProperty->GetName()),
				PreviousValue,
				UnderlyingProperty->GetSignedIntPropertyValue(InValue)
			));
	}
	else
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetProperty_Failed",
					"[{0}][{1}]Property:'{2}' operation failure."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
	}

	return Result;
}

bool FAruPredicate_SetObjectValue::Execute(
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

	const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(InProperty);
	if (ObjectProperty == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetObjectProperty_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not an uobject property."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	const UClass* ClassType = ObjectProperty->PropertyClass;
	if (ClassType == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetObjectProperty_ObjectClassNULL",
					"[{0}][{1}]Property:'{2}' is not a valid uobject property."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Error),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	bool Result = false;
	const UObject* PreviousValue = ObjectProperty->GetObjectPropertyValue(InValue);
	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<UObject*, EPropertyBagResult> ParameterValue = InParameters.GetValueObject(FName{ResolvedParameterName});
		if (!ParameterValue.HasValue())
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
				FText::Format(
					LOCTEXT(
						"NoPropertyFoundInParameter",
						"[{0}][{1}]Property:'{2}'. Can't find '{3}' in parameters."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Failed),
					FText::FromString(InProperty->GetName()),
					FText::FromString(ResolvedParameterName)
				));
			return false;
		}
		
		const UObject* ObjectPtr = ParameterValue.GetValue();
		if (ObjectPtr == nullptr)
		{
			ObjectProperty->SetObjectPropertyValue(InValue, nullptr);
		}
		else
		{
			const UClass* ObjectClass = ObjectPtr->GetClass();
			if (!ensure(ObjectClass != nullptr))
			{
				return false;
			}

			if (!ObjectClass->IsChildOf(ClassType))
			{
				FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetObjectProperty_ObjectClassMismatch",
					"Property:'{0}' is not a valid uobject property."),
				FText::FromString(InProperty->GetName())
			));
				return false;
			}

			ObjectProperty->SetObjectPropertyValue(InValue, ParameterValue.GetValue());
		}
		
		Result = true;
	}
	else if (auto* PendingValue = GetNewValueBySourceType<FObjectPropertyBase>(InParameters, ClassType).GetPtrOrNull())
	{
		ObjectProperty->CopyCompleteValue(InValue, *PendingValue);
		Result = true;
	}

	if (Result == true)
	{
		const UObject* InNewValue = ObjectProperty->GetObjectPropertyValue(InValue);
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
			FText::Format(
				LOCTEXT(
					"SetUObjectProperty_Success",
					"[{0}][{1}]Property:'{2}' previous value:{3}, new value:{4}"),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Success),
				FText::FromString(InProperty->GetName()),
				FText::FromString(!!PreviousValue? PreviousValue->GetName() : FString{"nullptr"}),
				FText::FromString(!!InNewValue? InNewValue->GetName() : FString{"nullptr"})
			));
	}
	else
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetProperty_Failed",
					"[{0}][{1}]Property:'{2}' operation failure."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
	}

	return Result;
}

bool FAruPredicate_SetStructValue::Execute(
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
					"SetStructProperty_PropertyTypeMismatch",
					"[{0}][{1}]Property:'{2}' is not a struct property."),
						FText::FromString(GetCompactName()),
						FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	const UScriptStruct* SourceStructType = StructProperty->Struct;
	if (SourceStructType == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
					FText::Format(
						LOCTEXT(
							"SetStructProperty_TypeNull",
							"[{0}][{1}]Property:'{2}' struct type is NULL."),
								FText::FromString(GetCompactName()),
								FText::FromString(Aru::ProcessResult::Failed),
						FText::FromString(InProperty->GetName())
					));
		return false;
	}
	
	if(SourceStructType == FInstancedStruct::StaticStruct())
	{
		
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
					FText::Format(
						LOCTEXT(
							"SetStructProperty_UnsupportedType",
							"[{0}][{1}]Property:'{2}' struct type is instanced struct, use 'Set Instanced Struct' instead."),
								FText::FromString(GetCompactName()),
								FText::FromString(Aru::ProcessResult::Failed),
						FText::FromString(InProperty->GetName())
					));
		return false;
	}

	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<FStructView, EPropertyBagResult> ParameterValue = InParameters.GetValueStruct(FName{ResolvedParameterName}, SourceStructType);
		if (!ParameterValue.HasValue())
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
				FText::Format(
					LOCTEXT(
						"NoPropertyFoundInParameter",
						"[{0}][{1}]Property:'{2}'. Can't find '{3}' in parameters."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Failed),
					FText::FromString(InProperty->GetName()),
					FText::FromString(ResolvedParameterName)
				));
			return false;
		}

		FStructView& StructValue = ParameterValue.GetValue();
		if (!StructValue.GetScriptStruct()->IsChildOf(SourceStructType))
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"SetStructProperty_StructTypeMismatch",
					"[{0}][{1}]Property:'{2}'.Source type:{3}, target type:{4}."),
						FText::FromString(GetCompactName()),
						FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName()),
				FText::FromString(SourceStructType->GetName()),
				FText::FromString(StructValue.GetScriptStruct()->GetName())
			));
			return false;
		}

		StructProperty->Struct->CopyScriptStruct(InValue, StructValue.GetMemory());
		return true;
	}

	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStructProperty>(InParameters, SourceStructType);
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

	const FInstancedStruct* InstancedStructPtr = static_cast<const FInstancedStruct*>(PendingValue);
	if (InstancedStructPtr == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"SetStructValue_InvalidValue",
					"[{0}][{1}]Property:'{2}': invalid struct value."),
						FText::FromString(GetCompactName()),
						FText::FromString(Aru::ProcessResult::Error)
			));
		return false;
	}

	const void* PendingStructValue = InstancedStructPtr->GetMemory();
	if (PendingStructValue == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Error(
			FText::Format(
				LOCTEXT(
					"SetStructValue_InvalidValue",
					"[{0}][{1}]Property:'{2}': invalid struct value."),
						FText::FromString(GetCompactName()),
						FText::FromString(Aru::ProcessResult::Error)
			));
		return false;
	}

	StructProperty->CopyCompleteValue(InValue, PendingStructValue);

	FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
			FText::Format(
				LOCTEXT(
					"SetStructValue_Result",
					"[{0}][{1}]Property:'{2}': operation succeeded."),
						FText::FromString(GetCompactName()),
						FText::FromString(Aru::ProcessResult::Success)
			));
	
	return true;
}

bool FAruPredicate_SetInstancedStructValue::Execute(
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

	const UScriptStruct* StructType = StructProperty->Struct;
	if (StructType == nullptr || StructType != FInstancedStruct::StaticStruct())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
					FText::Format(
						LOCTEXT(
							"SetInstancedStructProperty_TypeNull",
							"[{0}][{1}]Property:'{2}' struct type is NULL or not an instanced struct."),
								FText::FromString(GetCompactName()),
								FText::FromString(Aru::ProcessResult::Failed),
						FText::FromString(InProperty->GetName())
					));
		return false;
	}

	FInstancedStruct* InstancedStructPtr = static_cast<FInstancedStruct*>(InValue);
	if (InstancedStructPtr == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
					FText::Format(
						LOCTEXT(
							"SetInstancedStructProperty_TypeNull",
							"[{0}][{1}]Property:'{2}' is an invalid instanced struct."),
								FText::FromString(GetCompactName()),
								FText::FromString(Aru::ProcessResult::Failed),
						FText::FromString(InProperty->GetName())
					));
		return false;
	}

	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<FStructView, EPropertyBagResult> ParameterValue = InParameters.GetValueStruct(
			FName{ResolvedParameterName}, FInstancedStruct::StaticStruct());
		if (!ParameterValue.HasValue())
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
				FText::Format(
					LOCTEXT(
						"NoPropertyFoundInParameter",
						"[{0}][{1}]Property:'{2}'. Can't find '{3}' in parameters."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Failed),
					FText::FromString(InProperty->GetName()),
					FText::FromString(ResolvedParameterName)
				));
			return false;
		}

		FStructView& StructValue = ParameterValue.GetValue();
		if (StructValue.GetScriptStruct() != FInstancedStruct::StaticStruct())
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
					FText::Format(
						LOCTEXT(
							"SetInstancedStructProperty_TypeNull",
							"[{0}][{1}]Property:'{2}' struct type is not an instanced struct."),
								FText::FromString(GetCompactName()),
								FText::FromString(Aru::ProcessResult::Failed),
						FText::FromString(InProperty->GetName())
					));
			return false;
		}

		StructProperty->Struct->CopyScriptStruct(InValue, StructValue.GetMemory());
		return true;
	}

	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStructProperty>(InParameters, StructType);
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
				"SetInstancedStructValue_Result",
				"[{0}][{1}]Property:'{2}': operation succeeded."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Success),
						FText::FromString(InProperty->GetName())
		));
	
	return true;
}

#undef LOCTEXT_NAMESPACE
