#include "AssetFilters/AruFilter_ByType.h"

bool FAruFilter_ByObjectType::IsConditionMet(FProperty* InProperty, void* InValue) const
{
	if(ObjectType == nullptr)
	{
		return !bInverseCondition;
	}
	
	if(InProperty == nullptr)
	{
		return bInverseCondition;
	}

	const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(InProperty);
	if(ObjectProperty == nullptr)
	{
		return bInverseCondition;
	}
	
	const UClass* ClassType = ObjectProperty->PropertyClass;
	if(ClassType == nullptr)
	{
		return bInverseCondition;
	}

	return (ClassType == ObjectType) ^ bInverseCondition;
}

bool FAruFilter_ByStructType::IsConditionMet(FProperty* InProperty, void* InValue) const
{
	if(StructType == nullptr)
	{
		return !bInverseCondition;
	}
	
	if(InProperty == nullptr || InValue == nullptr)
	{
		return bInverseCondition;
	}
	
	const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty);
	if(StructProperty == nullptr)
	{
		return bInverseCondition;
	}

	const UScriptStruct* InStructType = StructProperty->Struct;
	if(InStructType == nullptr)
	{
		return bInverseCondition;
	}

	if(InStructType != FInstancedStruct::StaticStruct())
	{
		return (InStructType == StructType) ^ bInverseCondition;
	}

	FInstancedStruct* InstancedStructPtr = static_cast<FInstancedStruct*>(InValue);  
	if(InstancedStructPtr == nullptr || !InstancedStructPtr->IsValid())  
	{             
		return bInverseCondition;  
	}

	return (InstancedStructPtr->GetScriptStruct() == StructType) ^ bInverseCondition;
}
