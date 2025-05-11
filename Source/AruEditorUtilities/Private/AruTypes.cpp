#include "AruTypes.h"

void FAruActionDefinition::Invoke(FProperty* InProperty, void* InValue) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}

	for(auto& Condition : ForEachCondition())
	{
		if(!Condition.IsConditionMet(InProperty, InValue))
		{
			return;
		}
	}

	for(auto& Predicate : ForEachPredicates())
	{
		Predicate.Execute(InProperty, InValue);
	}
}
