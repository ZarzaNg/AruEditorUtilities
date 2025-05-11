#include "AssetPredicates/AruPredicate_PropertySetter.h"
#include "AruFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "FAruEditorUtilitiesModule"

void FAruPredicate_SetBoolValue::Execute(const FProperty* InProperty, void* InValue) const
{
	SetPropertyValue<FBoolProperty>(InProperty, InValue);
}

void FAruPredicate_SetFloatValue::Execute(const FProperty* InProperty, void* InValue) const
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
	
	SetPropertyValue<FNumericProperty>(InProperty, InValue);
}

void FAruPredicate_SetIntegerValue::Execute(const FProperty* InProperty, void* InValue) const
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

	SetPropertyValue<FNumericProperty>(InProperty, InValue);
}

void FAruPredicate_SetStringValue::Execute(const FProperty* InProperty, void* InValue) const
{
	SetPropertyValue<FStrProperty>(InProperty, InValue);
}

void FAruPredicate_SetTextValue::Execute(const FProperty* InProperty, void* InValue) const
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

	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStrProperty>();
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

void FAruPredicate_SetNameValue::Execute(const FProperty* InProperty, void* InValue) const
{
	SetPropertyValue<FNameProperty>(InProperty, InValue);
}

void FAruPredicate_SetEnumValue::Execute(const FProperty* InProperty, void* InValue) const
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

	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStrProperty>();
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

void FAruPredicate_SetObjectValue::Execute(const FProperty* InProperty, void* InValue) const
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

	if(auto* PendingValue = GetNewValueBySourceType<FObjectPropertyBase>(ClassType).GetPtrOrNull())
	{
		ObjectProperty->CopyCompleteValue(InValue, *PendingValue);
	}
}

void FAruPredicate_SetStructValue::Execute(const FProperty* InProperty, void* InValue) const
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

	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStructProperty>(SourceStructType);
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

void FAruPredicate_SetInstancedStructValue::Execute(const FProperty* InProperty, void* InValue) const
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
	
	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FStructProperty>(StructType);
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