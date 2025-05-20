#include "AssetFilters/AruFilter_ByName.h"

bool FAruFilter_ByName::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr)
	{
		return bInverseCondition;
	}

	if(CompareOp == EAruNameCompareOp::MatchAll)
	{
		return (PropertyName == InProperty->GetName() || PropertyName == InProperty->GetDisplayNameText().ToString()) ^ bInverseCondition;
	}
	else
	{
		return (InProperty->GetName().Contains(PropertyName) || InProperty->GetDisplayNameText().ToString().Contains(PropertyName)) ^ bInverseCondition;
	}
}

bool FAruFilter_ByObjectName::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return bInverseCondition;
	}
	
	const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(InProperty);
	if(ObjectProperty == nullptr)
	{
		return bInverseCondition;
	}

	UObject* ObjectPtr = ObjectProperty->GetObjectPropertyValue(InValue);
	if(ObjectPtr == nullptr)
	{
		return bInverseCondition;
	}

	if(CompareOp == EAruNameCompareOp::MatchAll)
	{
		return (ObjectName == ObjectPtr->GetName()) ^ bInverseCondition;
	}
	else
	{
		return (ObjectPtr->GetName().Contains(ObjectName)) ^ bInverseCondition;
	}
}
