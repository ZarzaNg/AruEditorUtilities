#include "AssetPredicates/AruPredicate_PropertySetter.h"
#include "UObject/PropertyAccessUtil.h"
#include "AruFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "FAruEditorUtilitiesModule"

void FAruPredicate_SetBoolValue::Execute(FProperty* InProperty, void* InValue) const
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

	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FAruPredicate_SetBoolValue>();
	if(!OptionalValue.IsSet())
	{
		return;
	}

	const void* PendingValue = OptionalValue.GetValue();
	if(PendingValue == nullptr)
	{
		return;
	}
	
	BoolProperty->CopyCompleteValue(InValue, PendingValue);
}

void FAruPredicate_SetFloatValue::Execute(FProperty* InProperty, void* InValue) const
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
	
	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FAruPredicate_SetFloatValue>();
	if(!OptionalValue.IsSet())
	{
		return;
	}

	const void* PendingValue = OptionalValue.GetValue();
	if(PendingValue == nullptr)
	{
		return;
	}
	
	NumericProperty->CopyCompleteValue(InValue, PendingValue);
}

void FAruPredicate_SetIntegerValue::Execute(FProperty* InProperty, void* InValue) const
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
	
	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FAruPredicate_SetIntegerValue>();
	if(!OptionalValue.IsSet())
	{
		return;
	}

	const void* PendingValue = OptionalValue.GetValue();
	if(PendingValue == nullptr)
	{
		return;
	}
	
	NumericProperty->CopyCompleteValue(InValue, PendingValue);
}

void FAruPredicate_SetNameValue::Execute(FProperty* InProperty, void* InValue) const
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
	
	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FAruPredicate_SetNameValue>();
	if(!OptionalValue.IsSet())
	{
		return;
	}

	const void* PendingValue = OptionalValue.GetValue();
	if(PendingValue == nullptr)
	{
		return;
	}
	
	NameProperty->CopyCompleteValue(InValue, PendingValue);
}

void FAruPredicate_SetObjectValue::Execute(FProperty* InProperty, void* InValue) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}

	const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(InProperty);
	if(ObjectProperty == nullptr)
	{
		return;
	}
	
	const UClass* ClassType = ObjectProperty->PropertyClass;
	if(ClassType == nullptr)
	{
		return;
	}

	if(auto* PendingValue = GetNewValueBySourceType<FAruPredicate_SetObjectValue>(ClassType).GetPtrOrNull())
	{
		ObjectProperty->CopyCompleteValue(InValue, *PendingValue);
	}
}

void FAruPredicate_SetStructValue::Execute(FProperty* InProperty, void* InValue) const
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

	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FAruPredicate_SetStructValue>(SourceStructType);
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

void FAruPredicate_SetInstancedStructValue::Execute(FProperty* InProperty, void* InValue) const
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
	
	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FAruPredicate_SetInstancedStructValue>(StructType);
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