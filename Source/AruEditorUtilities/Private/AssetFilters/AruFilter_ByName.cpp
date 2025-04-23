#include "AssetFilters/AruFilter_ByName.h"

bool FAruFilter_ByName::IsConditionMet(FProperty* InProperty, void* InContainer, void* InValue) const
{
	if(PropertyName == "None")
	{
		return !bInverseCondition;
	}
	
	if(InProperty == nullptr)
	{
		return bInverseCondition;
	}
	return (PropertyName == InProperty->GetName()) ^ bInverseCondition;
}
