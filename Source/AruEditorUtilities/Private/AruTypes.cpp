#include "AruTypes.h"

#include "AruFunctionLibrary.h"

void FAruActionDefinition::Invoke(FProperty* InProperty, void* InContainer, void* InValue) const
{
	if(InProperty == nullptr || InContainer == nullptr || InValue == nullptr)
	{
		return;
	}

	for(auto& Condition : ForEachCondition())
	{
		if(!Condition.IsConditionMet(InProperty, InContainer, InValue))
		{
			return;
		}
	}

	for(auto& Predicate : ForEachPredicates())
	{
		Predicate.Execute(InProperty, InContainer, InValue);
	}
}
