#include "AssetPredicates/AruPredicate_PropertySetter.h"

#define LOCTEXT_NAMESPACE "FAruEditorUtilitiesModule"

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
		// TODO: Add Warning.
		return {};
	}

	const void* PropertyValue = TargetProperty->ContainerPtrToValuePtr<void>(this);
	if (!IsCompatibleType(TargetProperty, PropertyValue, SourceType))
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Type mismatches", "Mismatch between target and source value types."));
		return {};
	}

	return TOptional<const void*>{PropertyValue};
}

TOptional<const void*> FAruPredicate_PropertySetter::GetValueFromObjectAsset(
	const FFieldClass* SourceProperty,
	const FInstancedPropertyBag& InParameters,
	const UStruct* SourceType) const
{
	if (PathToProperty.IsEmpty() || Object == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Find value failed", "Please check 'PathToProperty' and 'Object' configs."));
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
	for(auto& Element : PropertyChain)
	{
		Element = UAruFunctionLibrary::ResolveParameterizedString(InParameters, Element);
	}
	const FString&& ResolvedPath = FString::Join(PropertyChain, TEXT("."));

	auto&& PropertyContext = UAruFunctionLibrary::FindPropertyByPath(NativeClass, NativeObject, ResolvedPath);
	if (!PropertyContext.IsValid())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT("Find value failed", "Can't find property by path: '{0}' in object: '{1}'."),
				FText::FromString(PathToProperty),
				FText::FromName(Object.GetFName())));
		return {};
	}

	if (!PropertyContext.PropertyPtr->IsA(SourceProperty))
	{
		// TODO: Add Warning.
		return {};
	}

	if (!IsCompatibleType(PropertyContext.PropertyPtr, PropertyContext.ValuePtr.GetValue(), SourceType))
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Type mismatches", "Mismatch between target and source value types."));
		return {};
	}

	return TOptional<const void*>{PropertyContext.ValuePtr.GetValue()};
}

TOptional<const void*> FAruPredicate_PropertySetter::GetValueFromDataTable(
	const FFieldClass* SourceProperty,
	const FInstancedPropertyBag& InParameters,
	const UStruct* SourceType) const
{
	if (RowName.IsEmpty() || PathToProperty.IsEmpty() || DataTable == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			LOCTEXT("Find value failed", "Please check 'PathToProperty' and 'Object' as well as 'DataTable' configs."));
		return {};
	}

	const FString&& ResolvedRowName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, RowName);
	uint8* const* RowStructPtr = DataTable->GetRowMap().Find(FName{ResolvedRowName});
	if (RowStructPtr == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT("Find value failed", "Can't find row: '{0}' in DataTable: '{1}'."),
				FText::FromString(ResolvedRowName),
				FText::FromName(DataTable.GetFName())));
		return {};
	}

	const uint8* RowStruct = *RowStructPtr;
	if (RowStruct == nullptr)
	{
		return {};
	}

	TArray<FString> PropertyChain;
	PathToProperty.ParseIntoArray(PropertyChain, TEXT("."), true);
	for(auto& Element : PropertyChain)
	{
		Element = UAruFunctionLibrary::ResolveParameterizedString(InParameters, Element);
	}
	const FString&& ResolvedPath = FString::Join(PropertyChain, TEXT("."));

	auto&& PropertyContext = UAruFunctionLibrary::FindPropertyByPath(DataTable->RowStruct, RowStruct, ResolvedPath);
	if (!PropertyContext.IsValid())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT("Find value failed", "Can't find property by path: '{0}' in struct: '{1}'."),
				FText::FromString(PathToProperty),
				FText::FromName(DataTable->RowStruct.GetFName())));
		return {};
	}

	if (!PropertyContext.PropertyPtr->IsA(SourceProperty))
	{
		// TODO: Add Warning.
		return {};
	}

	if (!IsCompatibleType(PropertyContext.PropertyPtr, PropertyContext.ValuePtr.GetValue(), SourceType))
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Type mismatches", "Mismatch between target and source value types."));
		return {};
	}

	return TOptional<const void*>{PropertyContext.ValuePtr.GetValue()};
}

bool FAruPredicate_SetBoolValue::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr || InValue == nullptr)
	{
		return false;
	}

	const FBoolProperty* BoolProperty = CastField<FBoolProperty>(InProperty);
	if (BoolProperty == nullptr)
	{
		return false;
	}

	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<bool, EPropertyBagResult> ParameterValue = InParameters.GetValueBool(FName{ResolvedParameterName});
		if (!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return false;
		}
		BoolProperty->SetPropertyValue(InValue, ParameterValue.GetValue());
		return true;
	}

	return SetPropertyValue<FBoolProperty>(InProperty, InValue, InParameters);
}

bool FAruPredicate_SetFloatValue::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr || InValue == nullptr)
	{
		return false;
	}

	const FNumericProperty* NumericProperty = CastField<FNumericProperty>(InProperty);
	if (NumericProperty == nullptr)
	{
		return false;
	}

	if (!NumericProperty->IsFloatingPoint())
	{
		return false;
	}

	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<double, EPropertyBagResult> ParameterValue = InParameters.GetValueDouble(FName{ResolvedParameterName});
		if (!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return false;
		}
		NumericProperty->SetFloatingPointPropertyValue(InValue, ParameterValue.GetValue());
		return true;
	}

	return SetPropertyValue<FNumericProperty>(InProperty, InValue, InParameters);
}

bool FAruPredicate_SetIntegerValue::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr || InValue == nullptr)
	{
		return false;
	}

	const FNumericProperty* NumericProperty = CastField<FNumericProperty>(InProperty);
	if (NumericProperty == nullptr)
	{
		return false;
	}

	if (!NumericProperty->IsInteger())
	{
		return false;
	}

	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<int64, EPropertyBagResult> ParameterValue = InParameters.GetValueInt64(FName{ResolvedParameterName});
		if (!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return false;
		}
		NumericProperty->SetIntPropertyValue(InValue, ParameterValue.GetValue());
		return true;
	}

	return SetPropertyValue<FNumericProperty>(InProperty, InValue, InParameters);
}

bool FAruPredicate_SetStringValue::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr || InValue == nullptr)
	{
		return false;
	}

	const FStrProperty* StrProperty = CastField<FStrProperty>(InProperty);
	if (StrProperty == nullptr)
	{
		return false;
	}

	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<FString, EPropertyBagResult> ParameterValue = InParameters.GetValueString(FName{ResolvedParameterName});
		if (!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return false;
		}
		StrProperty->SetPropertyValue(InValue, ParameterValue.GetValue());
		return true;
	}

	return SetPropertyValue<FStrProperty>(InProperty, InValue, InParameters);
}

bool FAruPredicate_SetTextValue::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr || InValue == nullptr)
	{
		return false;
	}

	const FTextProperty* TextProperty = CastField<FTextProperty>(InProperty);
	if (TextProperty == nullptr)
	{
		return false;
	}

	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<FText, EPropertyBagResult> ParameterValue = InParameters.GetValueText(FName{ResolvedParameterName});
		if (!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return false;
		}
		TextProperty->SetPropertyValue(InValue, ParameterValue.GetValue());
		return true;
	}

	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStrProperty>(InParameters);
	if (!OptionalValue.IsSet())
	{
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
	return true;
}

bool FAruPredicate_SetNameValue::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr || InValue == nullptr)
	{
		return false;
	}

	const FNameProperty* NameProperty = CastField<FNameProperty>(InProperty);
	if (NameProperty == nullptr)
	{
		return false;
	}

	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<FName, EPropertyBagResult> ParameterValue = InParameters.GetValueName(FName{ResolvedParameterName});
		if (!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return false;
		}
		NameProperty->SetPropertyValue(InValue, ParameterValue.GetValue());
		return true;
	}

	return SetPropertyValue<FNameProperty>(InProperty, InValue, InParameters);
}

bool FAruPredicate_SetEnumValue::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr || InValue == nullptr)
	{
		return false;
	}

	const FEnumProperty* EnumProperty = CastField<FEnumProperty>(InProperty);
	if (EnumProperty == nullptr)
	{
		return false;
	}

	const UEnum* EnumType = EnumProperty->GetEnum();
	if (EnumType == nullptr)
	{
		return false;
	}

	const FNumericProperty* UnderlyingProperty = EnumProperty->GetUnderlyingProperty();
	if (UnderlyingProperty == nullptr)
	{
		return false;
	}

	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<uint8, EPropertyBagResult> ParameterValue = InParameters.GetValueEnum(FName{ResolvedParameterName}, EnumType);
		if (!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return false;
		}
		UnderlyingProperty->SetIntPropertyValue(InValue, static_cast<int64>(ParameterValue.GetValue()));
		return true;
	}

	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStrProperty>(InParameters);
	if (!OptionalValue.IsSet())
	{
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
		// TODO: Add warning.
		return false;
	}

	UnderlyingProperty->SetIntPropertyValue(InValue, PendingEnumValue);
	return true;
}

bool FAruPredicate_SetObjectValue::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr || InValue == nullptr)
	{
		return false;
	}

	const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(InProperty);
	if (ObjectProperty == nullptr)
	{
		return false;
	}

	const UClass* ClassType = ObjectProperty->PropertyClass;
	if (ClassType == nullptr)
	{
		return false;
	}

	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<UObject*, EPropertyBagResult> ParameterValue = InParameters.GetValueObject(FName{ResolvedParameterName});
		if (!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return false;
		}
		const UObject* ObjectPtr = ParameterValue.GetValue();
		if (ObjectPtr == nullptr)
		{
			ObjectProperty->SetObjectPropertyValue(InValue, nullptr);
			return false;
		}

		const UClass* ObjectClass = ObjectPtr->GetClass();
		if (!ensure(ObjectClass != nullptr))
		{
			return false;
		}

		if (!ObjectClass->IsChildOf(ClassType))
		{
			// TODO: Add Type mismatch log.
			return false;
		}

		ObjectProperty->SetObjectPropertyValue(InValue, ParameterValue.GetValue());
		return true;
	}

	if (auto* PendingValue = GetNewValueBySourceType<FObjectPropertyBase>(InParameters, ClassType).GetPtrOrNull())
	{
		ObjectProperty->CopyCompleteValue(InValue, *PendingValue);
		return true;
	}

	return false;
}

bool FAruPredicate_SetStructValue::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr || InValue == nullptr)
	{
		return false;
	}

	const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty);
	if (StructProperty == nullptr)
	{
		return false;
	}

	const UScriptStruct* SourceStructType = StructProperty->Struct;
	if (SourceStructType == nullptr || SourceStructType == FInstancedStruct::StaticStruct())
	{
		return false;
	}

	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<FStructView, EPropertyBagResult> ParameterValue = InParameters.GetValueStruct(FName{ResolvedParameterName}, SourceStructType);
		if (!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return false;
		}

		FStructView& StructValue = ParameterValue.GetValue();
		if (!StructValue.GetScriptStruct()->IsChildOf(SourceStructType))
		{
			// TODO: Add Type mismatch log.
			return false;
		}

		StructProperty->Struct->CopyScriptStruct(InValue, StructValue.GetMemory());
		return true;
	}

	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStructProperty>(InParameters, SourceStructType);
	if (!OptionalValue.IsSet())
	{
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
		return false;
	}

	const void* PendingStructValue = InstancedStructPtr->GetMemory();
	if (PendingStructValue == nullptr)
	{
		return false;
	}

	StructProperty->CopyCompleteValue(InValue, PendingStructValue);
	return true;
}

bool FAruPredicate_SetInstancedStructValue::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr || InValue == nullptr)
	{
		return false;
	}

	const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty);
	if (StructProperty == nullptr)
	{
		return false;
	}

	const UScriptStruct* StructType = StructProperty->Struct;
	if (StructType == nullptr || StructType != FInstancedStruct::StaticStruct())
	{
		return false;
	}

	FInstancedStruct* InstancedStructPtr = static_cast<FInstancedStruct*>(InValue);
	if (InstancedStructPtr == nullptr)
	{
		return false;
	}

	if (ValueSource == EAruValueSource::Parameters)
	{
		const FString&& ResolvedParameterName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ParameterName);
		TValueOrError<FStructView, EPropertyBagResult> ParameterValue = InParameters.GetValueStruct(FName{ResolvedParameterName}, FInstancedStruct::StaticStruct());
		if (!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return false;
		}

		FStructView& StructValue = ParameterValue.GetValue();
		if (StructValue.GetScriptStruct() != FInstancedStruct::StaticStruct())
		{
			// TODO: Add Type mismatch log.
			return false;
		}

		StructProperty->Struct->CopyScriptStruct(InValue, StructValue.GetMemory());
		return true;
	}

	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStructProperty>(InParameters, StructType);
	if (!OptionalValue.IsSet())
	{
		return false;
	}

	const void* PendingValue = OptionalValue.GetValue();
	if (PendingValue == nullptr)
	{
		return false;
	}

	StructProperty->CopyCompleteValue(InValue, PendingValue);
	return true;
}

#undef LOCTEXT_NAMESPACE
