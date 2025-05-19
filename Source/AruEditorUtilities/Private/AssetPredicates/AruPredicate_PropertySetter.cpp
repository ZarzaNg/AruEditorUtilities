#include "AssetPredicates/AruPredicate_PropertySetter.h"

#define LOCTEXT_NAMESPACE "FAruEditorUtilitiesModule"

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
		BoolProperty->CopyCompleteValue(InValue, &ParameterValue.GetValue());
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