#include "AssetPredicates/AruPredicate_PropertySetter.h"
#include "UObject/PropertyAccessUtil.h"
#include "AruFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "FAruEditorUtilitiesModule"

void FAruPredicate_SetBoolValue::Execute(const FProperty* InProperty, void* InValue) const
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

	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FBoolProperty>();
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
	
	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FNumericProperty>();
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
	
	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FNumericProperty>();
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

void FAruPredicate_SetNameValue::Execute(const FProperty* InProperty, void* InValue) const
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
	
	TOptional<const void*> OptionalValue = GetNewValueBySourceType<FNameProperty>();
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
	
	StructProperty->CopyCompleteValue(InValue, PendingValue);
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