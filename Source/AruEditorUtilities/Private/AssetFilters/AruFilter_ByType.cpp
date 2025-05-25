#include "AssetFilters/AruFilter_ByType.h"

bool FAruFilter_ByObjectType::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if (ObjectType == nullptr)
	{
		return !bInverseCondition;
	}

	if (InProperty == nullptr)
	{
		return bInverseCondition;
	}

	const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(InProperty);
	if (ObjectProperty == nullptr)
	{
		return bInverseCondition;
	}

	const UClass* ClassType = ObjectProperty->PropertyClass;
	if (ClassType == nullptr)
	{
		return bInverseCondition;
	}

	return ObjectType->IsChildOf(ClassType) ^ bInverseCondition;
}

bool FAruFilter_ByStructType::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if (StructType == nullptr)
	{
		return !bInverseCondition;
	}

	if (InProperty == nullptr)
	{
		return bInverseCondition;
	}

	const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty);
	if (StructProperty == nullptr)
	{
		return bInverseCondition;
	}

	const UScriptStruct* InStructType = StructProperty->Struct;
	if (InStructType == nullptr)
	{
		return bInverseCondition;
	}

	return StructType->IsChildOf(InStructType) ^ bInverseCondition;
}

bool FAruFilter_ByInstancedStructType::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if (StructType == nullptr)
	{
		return !bInverseCondition;
	}

	if (InProperty == nullptr || InValue == nullptr)
	{
		return bInverseCondition;
	}

	const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty);
	if (StructProperty == nullptr)
	{
		return bInverseCondition;
	}

	const UScriptStruct* InStructType = StructProperty->Struct;
	if (InStructType != FInstancedStruct::StaticStruct())
	{
		return bInverseCondition;
	}

	const FInstancedStruct* InstancedStructPtr = static_cast<const FInstancedStruct*>(InValue);
	if (InstancedStructPtr == nullptr)
	{
		return bInverseCondition;
	}

	const UScriptStruct* NativeStructType = InstancedStructPtr->GetScriptStruct();
	if (NativeStructType == nullptr)
	{
		return bInverseCondition;
	}

	return StructType->IsChildOf(NativeStructType) ^ bInverseCondition;
}
