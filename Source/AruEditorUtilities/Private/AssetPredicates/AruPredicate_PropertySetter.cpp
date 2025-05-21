#include "AssetPredicates/AruPredicate_PropertySetter.h"

#define LOCTEXT_NAMESPACE "FAruEditorUtilitiesModule"

bool FAruPredicate_PropertySetter::IsCompatibleType(
	const FProperty* TargetProperty,
	const void* TargetValue,
	const UStruct* SourceType)
{
	if(TargetProperty == nullptr || SourceType == nullptr)
	{
		return true;
	}

	if(const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(TargetProperty))
	{
		return IsCompatibleObjectType(ObjectProperty, TargetValue, SourceType);
	}
			
	if(const FStructProperty* StructProperty = CastField<FStructProperty>(TargetProperty))
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
	if(Object == nullptr)
	{
		// We might want to clear the property value.
		// So when we got nullptr, we consider it as matched.
		return true;
	}
				
	const UClass* ObjectClass = Object->GetClass();
	if(ObjectClass == nullptr)
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
	if(ScriptStruct == nullptr)
	{
		return false;
	}

	if(SourceType == FInstancedStruct::StaticStruct())
	{
		return ScriptStruct == FInstancedStruct::StaticStruct();
	}

	const FInstancedStruct* InstancedStructPtr = static_cast<const FInstancedStruct*>(TargetValue);  
	if(InstancedStructPtr == nullptr)  
	{             
		return false;  
	}

	const UScriptStruct* InnerStruct = InstancedStructPtr->GetScriptStruct();
	if(InnerStruct == nullptr)
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
	if(!ensureMsgf(TargetProperty != nullptr, TEXT("Can't find NewValue property in derived struct.")))
	{
		return {};
	}

	if(!TargetProperty->IsA(SourceProperty))
	{
		// TODO: Add Warning.
		return {};
	}
				
	const void* PropertyValue = TargetProperty->ContainerPtrToValuePtr<void>(this);
	if(!IsCompatibleType(TargetProperty, PropertyValue, SourceType))
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Type mismatches", "Mismatch between target and source value types."));
		return {};
	}
				
	return TOptional<const void*>{PropertyValue};
}

TOptional<const void*> FAruPredicate_PropertySetter::GetValueFromObjectAsset(
	const FFieldClass* SourceProperty,
	const UStruct* SourceType) const
{
	if(PathToProperty.IsEmpty() || Object == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Find value failed", "Please check 'PathToProperty' and 'Object' configs."));
		return {};
	}

	const UClass* NativeClass = Object.GetClass();
	const UObject* NativeObject = Object;
	if(UBlueprint* BlueprintObject = Cast<UBlueprint>(Object))
	{
		NativeClass = BlueprintObject->GeneratedClass;
		NativeObject= NativeClass->GetDefaultObject();
	}

	auto&& PropertyContext = UAruFunctionLibrary::FindPropertyByPath(NativeClass, NativeObject, PathToProperty);
	if(!PropertyContext.IsValid())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(LOCTEXT("Find value failed", "Can't find property by path: '{0}' in object: '{1}'."),
				FText::FromString(PathToProperty),
				FText::FromName(Object.GetFName())));
		return {};
	}

	if(!PropertyContext.PropertyPtr->IsA(SourceProperty))
	{
		// TODO: Add Warning.
		return {};
	}

	if(!IsCompatibleType(PropertyContext.PropertyPtr, PropertyContext.ValuePtr.GetValue(), SourceType))
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
	if(RowName.IsEmpty() || PathToProperty.IsEmpty() || DataTable == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Find value failed", "Please check 'PathToProperty' and 'Object' as well as 'DataTable' configs."));
		return {};
	}
	
	FName InternalRowName{RowName};
	if(RowName[0] == '@')
	{
		TValueOrError<FName, EPropertyBagResult> SearchNameResult = InParameters.GetValueName(FName{RowName.RightChop(1)});
		if(SearchNameResult.HasValue())
		{
			InternalRowName = SearchNameResult.GetValue();
		}
		else
		{
			TValueOrError<FString, EPropertyBagResult> SearchStringResult = InParameters.GetValueString(FName{RowName.RightChop(1)});
			if(SearchStringResult.HasValue())
			{
				InternalRowName = FName{SearchStringResult.GetValue()};
			}
		}
	}
	
	uint8* const* RowStructPtr = DataTable->GetRowMap().Find(FName{InternalRowName});
	if(RowStructPtr == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(LOCTEXT("Find value failed", "Can't find row: '{0}' in DataTable: '{1}'."),
				FText::FromName(InternalRowName),
				FText::FromName(DataTable.GetFName())));
		return {};
	}

	const uint8* RowStruct = *RowStructPtr;
	if(RowStruct == nullptr)
	{
		return {};
	}

	auto&& PropertyContext = UAruFunctionLibrary::FindPropertyByPath(DataTable->RowStruct, RowStruct, PathToProperty);
	if(!PropertyContext.IsValid())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(LOCTEXT("Find value failed", "Can't find property by path: '{0}' in struct: '{1}'."),
				FText::FromString(PathToProperty),
				FText::FromName(DataTable->RowStruct.GetFName())));
		return {};
	}

	if(!PropertyContext.PropertyPtr->IsA(SourceProperty))
	{
		// TODO: Add Warning.
		return {};
	}

	if(!IsCompatibleType(PropertyContext.PropertyPtr, PropertyContext.ValuePtr.GetValue(), SourceType))
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Type mismatches", "Mismatch between target and source value types."));
		return {};
	}

	return TOptional<const void*>{PropertyContext.ValuePtr.GetValue()};
}

void FAruPredicate_SetBoolValue::Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}

	const FBoolProperty* BoolProperty = CastField<FBoolProperty>(InProperty);
	if(BoolProperty == nullptr)
	{
		return;
	}

	if(ValueSource == EAruValueSource::Parameters)
	{
		TValueOrError<bool, EPropertyBagResult> ParameterValue = InParameters.GetValueBool(FName{ParameterName});
		if(!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return;
		}
		BoolProperty->SetPropertyValue(InValue, ParameterValue.GetValue());
	}
	else
	{
		SetPropertyValue<FBoolProperty>(InProperty, InValue, InParameters);
	}
}

void FAruPredicate_SetFloatValue::Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}

	const FNumericProperty* NumericProperty = CastField<FNumericProperty>(InProperty);
	if(NumericProperty == nullptr)
	{
		return;
	}
	
	if(!NumericProperty->IsFloatingPoint())
	{
		return;
	}
	
	if(ValueSource == EAruValueSource::Parameters)
	{
		TValueOrError<double, EPropertyBagResult> ParameterValue = InParameters.GetValueDouble(FName{ParameterName});
		if(!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return;
		}
		NumericProperty->SetFloatingPointPropertyValue(InValue, ParameterValue.GetValue());
	}
	else
	{
		SetPropertyValue<FNumericProperty>(InProperty, InValue, InParameters);
	}
}

void FAruPredicate_SetIntegerValue::Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}

	const FNumericProperty* NumericProperty = CastField<FNumericProperty>(InProperty);
	if(NumericProperty == nullptr)
	{
		return;
	}

	if(!NumericProperty->IsInteger())
	{
		return;
	}

	if(ValueSource == EAruValueSource::Parameters)
	{
		TValueOrError<int64, EPropertyBagResult> ParameterValue = InParameters.GetValueInt64(FName{ParameterName});
		if(!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return;
		}
		NumericProperty->SetIntPropertyValue(InValue, ParameterValue.GetValue());
	}
	else
	{
		SetPropertyValue<FNumericProperty>(InProperty, InValue, InParameters);
	}
}

void FAruPredicate_SetStringValue::Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}

	const FStrProperty* StrProperty = CastField<FStrProperty>(InProperty);
	if(StrProperty == nullptr)
	{
		return;
	}

	if(ValueSource == EAruValueSource::Parameters)
	{
		TValueOrError<FString, EPropertyBagResult> ParameterValue = InParameters.GetValueString(FName{ParameterName});
		if(!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return;
		}
		StrProperty->SetPropertyValue(InValue, ParameterValue.GetValue());
	}
	else
	{
		SetPropertyValue<FStrProperty>(InProperty, InValue, InParameters);
	}
}

void FAruPredicate_SetTextValue::Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}

	const FTextProperty* TextProperty = CastField<FTextProperty>(InProperty);
	if(TextProperty == nullptr)
	{
		return;
	}

	if(ValueSource == EAruValueSource::Parameters)
	{
		TValueOrError<FText, EPropertyBagResult> ParameterValue = InParameters.GetValueText(FName{ParameterName});
		if(!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return;
		}
		TextProperty->SetPropertyValue(InValue, ParameterValue.GetValue());
	}
	else
	{
		TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStrProperty>(InParameters);
		if(!OptionalValue.IsSet())
		{
			return;
		}

		const void* PendingValue = OptionalValue.GetValue();
		if(PendingValue == nullptr)
		{
			return;
		}

		const FString* StringValue = static_cast<const FString*>(PendingValue);
		if(StringValue == nullptr)
		{
			return;
		}
	
		FText PendingTextValue = FText::FromString(*StringValue);
		TextProperty->CopyCompleteValue(InValue, &PendingTextValue);
	}
}

void FAruPredicate_SetNameValue::Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}

	const FNameProperty* NameProperty = CastField<FNameProperty>(InProperty);
	if(NameProperty == nullptr)
	{
		return;
	}

	if(ValueSource == EAruValueSource::Parameters)
	{
		TValueOrError<FName, EPropertyBagResult> ParameterValue = InParameters.GetValueName(FName{ParameterName});
		if(!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return;
		}
		NameProperty->SetPropertyValue(InValue, ParameterValue.GetValue());
	}
	else
	{
		SetPropertyValue<FNameProperty>(InProperty, InValue, InParameters);
	}
}

void FAruPredicate_SetEnumValue::Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}

	const FEnumProperty* EnumProperty = CastField<FEnumProperty>(InProperty);
	if(EnumProperty == nullptr)
	{
		return;
	}

	const UEnum* EnumType = EnumProperty->GetEnum();
	if(EnumType == nullptr)
	{
		return;
	}

	const FNumericProperty* UnderlyingProperty = EnumProperty->GetUnderlyingProperty();
	if(UnderlyingProperty == nullptr)
	{
		return;
	}

	if(ValueSource == EAruValueSource::Parameters)
	{
		TValueOrError<uint8, EPropertyBagResult> ParameterValue = InParameters.GetValueEnum(FName{ParameterName}, EnumType);
		if(!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return;
		}
		UnderlyingProperty->SetIntPropertyValue(InValue, static_cast<int64>(ParameterValue.GetValue()));
	}
	else
	{
		TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStrProperty>(InParameters);
		if(!OptionalValue.IsSet())
		{
			return;
		}

		const void* PendingValue = OptionalValue.GetValue();
		if(PendingValue == nullptr)
		{
			return;
		}

		const FString* StringValue = static_cast<const FString*>(PendingValue);
		if(StringValue == nullptr)
		{
			return;
		}

		const int64 PendingEnumValue = EnumType->GetValueByNameString(*StringValue);
		if(PendingEnumValue == INDEX_NONE)
		{
			// TODO: Add warning.
			return;
		}

		UnderlyingProperty->SetIntPropertyValue(InValue, PendingEnumValue);
	}
}

void FAruPredicate_SetObjectValue::Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}

	const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(InProperty);
	if(ObjectProperty == nullptr)
	{
		return;
	}
	
	const UClass* ClassType = ObjectProperty->PropertyClass;
	if(ClassType == nullptr)
	{
		return;
	}
	
	if(ValueSource == EAruValueSource::Parameters)
	{
		TValueOrError<UObject*, EPropertyBagResult> ParameterValue = InParameters.GetValueObject(FName{ParameterName});
		if(!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return;
		}
		const UObject* ObjectPtr = ParameterValue.GetValue();
		if(ObjectPtr == nullptr)
		{
			ObjectProperty->SetObjectPropertyValue(InValue, nullptr);
			return;
		}
		
		const UClass* ObjectClass = ObjectPtr->GetClass();
		if(ensure(ObjectClass == nullptr))
		{
			return;
		}

		if(!ObjectClass->IsChildOf(ClassType))
		{
			// TODO: Add Type mismatch log.
			return;
		}
		
		ObjectProperty->SetObjectPropertyValue(InValue, ParameterValue.GetValue());
	}
	else if(auto* PendingValue = GetNewValueBySourceType<FObjectPropertyBase>(InParameters, ClassType).GetPtrOrNull())
	{
		ObjectProperty->CopyCompleteValue(InValue, *PendingValue);
	}
}

void FAruPredicate_SetStructValue::Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}
	
	const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty);
	if(StructProperty == nullptr)
	{
		return;
	}

	const UScriptStruct* SourceStructType = StructProperty->Struct;
	if(SourceStructType == nullptr || SourceStructType == FInstancedStruct::StaticStruct())  
	{          
		return;  
	}

	if(ValueSource == EAruValueSource::Parameters)
	{
		TValueOrError<FStructView, EPropertyBagResult> ParameterValue = InParameters.GetValueStruct(FName{ParameterName}, SourceStructType);
		if(!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return;
		}
		
		FStructView& StructValue = ParameterValue.GetValue();
		if(!StructValue.GetScriptStruct()->IsChildOf(SourceStructType))
		{
			// TODO: Add Type mismatch log.
			return;
		}
		
		StructProperty->Struct->CopyScriptStruct(InValue, StructValue.GetMemory());
	}
	else
	{
		TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStructProperty>(InParameters, SourceStructType);
		if(!OptionalValue.IsSet())
		{
			return;
		}

		const void* PendingValue = OptionalValue.GetValue();
		if(PendingValue == nullptr)
		{
			return;
		}

		const FInstancedStruct* InstancedStructPtr = static_cast<const FInstancedStruct*>(PendingValue);
		if(InstancedStructPtr == nullptr)
		{
			return;
		}

		const void* PendingStructValue = InstancedStructPtr->GetMemory();
		if(PendingStructValue == nullptr)
		{
			return;
		}
	
		StructProperty->CopyCompleteValue(InValue, PendingStructValue);
	}
}

void FAruPredicate_SetInstancedStructValue::Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}
	
	const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty);
	if(StructProperty == nullptr)
	{
		return;
	}

	const UScriptStruct* StructType = StructProperty->Struct;  
	if(StructType == nullptr || StructType != FInstancedStruct::StaticStruct())  
	{          
		return;  
	}

	FInstancedStruct* InstancedStructPtr = static_cast<FInstancedStruct*>(InValue);  
	if(InstancedStructPtr == nullptr)  
	{             
		return;  
	}

	if(ValueSource == EAruValueSource::Parameters)
	{
		TValueOrError<FStructView, EPropertyBagResult> ParameterValue = InParameters.GetValueStruct(FName{ParameterName}, FInstancedStruct::StaticStruct());
		if(!ParameterValue.HasValue())
		{
			// TODO: Add log.
			return;
		}
		
		FStructView& StructValue = ParameterValue.GetValue();
		if(StructValue.GetScriptStruct() != FInstancedStruct::StaticStruct())
		{
			// TODO: Add Type mismatch log.
			return;
		}
		
		StructProperty->Struct->CopyScriptStruct(InValue, StructValue.GetMemory());
	}
	
	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStructProperty>(InParameters, StructType);
	if(!OptionalValue.IsSet())
	{
		return;
	}

	const void* PendingValue = OptionalValue.GetValue();
	if(PendingValue == nullptr)
	{
		return;
	}
	
	StructProperty->CopyCompleteValue(InValue, PendingValue);
}

#undef LOCTEXT_NAMESPACE