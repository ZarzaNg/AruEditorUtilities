#include "AruTypes.h"

void FAruActionDefinition::Invoke(FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}

	for(auto& Condition : ForEachCondition())
	{
		if(!Condition.IsConditionMet(InProperty, InValue, InParameters))
		{
			return;
		}
	}

	for(auto& Predicate : ForEachPredicates())
	{
		Predicate.Execute(InProperty, InValue, InParameters);
	}
}
