#include "AssetPredicates/AruPredicate_PropertySetter.h"
#include "UObject/PropertyAccessUtil.h"
#include "AruFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "FAruEditorUtilitiesModule"

void FAruPredicate_SetBoolValue::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}

	if(const FBoolProperty* BoolProperty = CastField<FBoolProperty>(InProperty))
	{
		if(auto* PendingValue = GetNewValueBySourceType<FAruPredicate_SetBoolValue>())
		{
			BoolProperty->CopyCompleteValue(InValue, PendingValue);
		}
	}
}

void FAruPredicate_SetFloatValue::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}
	
	if(const FNumericProperty* NumericProperty = CastField<FNumericProperty>(InProperty))
	{
		if(!NumericProperty->IsFloatingPoint())
		{
			return;
		}
		
		if(auto* PendingValue = GetNewValueBySourceType<FAruPredicate_SetFloatValue>())
		{
			NumericProperty->CopyCompleteValue(InValue, PendingValue);
		}
	}
}

void FAruPredicate_SetIntegerValue::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}
	
	if(const FNumericProperty* NumericProperty = CastField<FNumericProperty>(InProperty))
	{
		if(!NumericProperty->IsInteger())
		{
			return;
		}

		if(auto* PendingValue = GetNewValueBySourceType<FAruPredicate_SetIntegerValue>())
		{
			NumericProperty->CopyCompleteValue(InValue, PendingValue);
		}
	}
}

void FAruPredicate_SetNameValue::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}

	if(const FNameProperty* NameProperty = CastField<FNameProperty>(InProperty))
	{
		if(auto* PendingValue = GetNewValueBySourceType<FAruPredicate_SetNameValue>())
		{
			NameProperty->CopyCompleteValue(InValue, PendingValue);
		}
	}
}

void FAruPredicate_SetObjectValue::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
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

	if(auto* PendingValue = GetNewValueBySourceType<FAruPredicate_SetObjectValue>(ClassType))
	{
		ObjectProperty->CopyCompleteValue(InValue, PendingValue);
	}
}

void FAruPredicate_SetStructValue::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
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

	if(auto* PendingValue = GetNewValueBySourceType<FAruPredicate_SetStructValue>(SourceStructType))
	{
		StructProperty->CopyCompleteValue(InValue, PendingValue);
	}
}

void FAruPredicate_SetInstancedStructValue::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
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

	if(auto* PendingValue = GetNewValueBySourceType<FAruPredicate_SetInstancedStructValue>(StructType))
	{
		StructProperty->CopyCompleteValue(InValue, PendingValue);
	}
}

#undef LOCTEXT_NAMESPACE