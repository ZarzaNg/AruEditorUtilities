#include "AssetPredicates/AruPredicate_PropertySetter.h"
#include "AruFunctionLibrary.h"

void FAruPredicate_SetBoolValue::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}

	if(const FBoolProperty* BoolProperty = CastField<FBoolProperty>(InProperty))
	{
		BoolProperty->SetPropertyValue(InValue, NewVal);
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
		NumericProperty->SetFloatingPointPropertyValue(InValue, NewVal);
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
		NumericProperty->SetIntPropertyValue(InValue, NewVal);
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
		NameProperty->SetPropertyValue(InValue, NewVal);
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

	const UScriptStruct* TargetStructType = NewVal.GetScriptStruct();
	if(TargetStructType == nullptr || TargetStructType == FInstancedStruct::StaticStruct())  
	{          
		return;  
	}

	if(SourceStructType != TargetStructType)
	{
		return;
	}

	SourceStructType->CopyScriptStruct(InValue, NewVal.GetMemory());
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

	if(NewVal != nullptr && !NewVal->GetClass()->IsChildOf(ClassType))
	{
		return;
	}

	ObjectProperty->SetObjectPtrPropertyValue(InValue, NewVal);
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

	*InstancedStructPtr = NewVal;
}
