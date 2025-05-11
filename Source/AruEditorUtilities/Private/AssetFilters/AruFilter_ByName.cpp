#include "AssetFilters/AruFilter_ByName.h"

bool FAruFilter_ByName::IsConditionMet(FProperty* InProperty, void* InValue) const
{
	if(InProperty == nullptr)
	{
		return bInverseCondition;
	}
	
	return (PropertyName == InProperty->GetName() || PropertyName == InProperty->GetDisplayNameText().ToString()) ^ bInverseCondition;
}
