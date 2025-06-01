#include "AruTypes.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AruTypes)

bool FAruActionDefinition::Invoke(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr || InValue == nullptr)
	{
		return false;
	}

	for (auto& Condition : ForEachCondition())
	{
		if (!Condition.IsConditionMet(InProperty, InValue, InParameters))
		{
			return false;
		}
	}

	bool bExecutedSuccessfully = false;
	for (auto& Predicate : ForEachPredicates())
	{
		bExecutedSuccessfully |= Predicate.Execute(InProperty, InValue, InParameters);
	}
	return bExecutedSuccessfully;
}
