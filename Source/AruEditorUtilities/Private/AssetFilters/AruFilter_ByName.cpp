#include "AssetFilters/AruFilter_ByName.h"

bool FAruFilter_ByName::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr)
	{
		return bInverseCondition;
	}
	
	return (PropertyName == InProperty->GetName() || PropertyName == InProperty->GetDisplayNameText().ToString()) ^ bInverseCondition;
}
