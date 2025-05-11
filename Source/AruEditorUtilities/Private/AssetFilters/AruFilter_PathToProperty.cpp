#include "AssetFilters/AruFilter_PathToProperty.h"

#include "AruFunctionLibrary.h"

bool FAruFilter_PathToProperty::IsConditionMet(FProperty* InProperty, void* InContainer, void* InValue) const
{
	if(PathToProperty.IsEmpty() || InValue == nullptr || !Filter.IsValid())
	{
		return bInverseCondition;
	}

	FAruPropertyContext PropertyContext = UAruFunctionLibrary::FindPropertyByPath(InProperty, InValue, PathToProperty);
	if(!PropertyContext.IsValid())
	{
		return bInverseCondition;
	}
	
	if(const FAruFilter* FilterPtr = Filter.GetPtr<FAruFilter>())
	{
		return FilterPtr->IsConditionMet(PropertyContext.PropertyPtr, nullptr, PropertyContext.ValuePtr.GetValue());
	}
	
	return bInverseCondition;
}
