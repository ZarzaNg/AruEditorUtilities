#include "AssetPredicates/AruPredicate_PathToProperty.h"
#include "AruFunctionLibrary.h"

void FAruPredicate_PathToProperty::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
{
	if(PathToProperty.IsEmpty() || InValue == nullptr || !Predicate.IsValid())
	{
		return;
	}
	
	FAruPropertyContext PropertyContext = UAruFunctionLibrary::FindPropertyByPath(InProperty, InValue, PathToProperty);
	if(const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>())
	{
		PredicatePtr->Execute(PropertyContext.PropertyPtr, nullptr, PropertyContext.ValuePtr);
	}
}
