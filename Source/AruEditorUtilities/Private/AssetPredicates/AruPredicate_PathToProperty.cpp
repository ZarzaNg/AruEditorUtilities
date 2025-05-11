#include "AssetPredicates/AruPredicate_PathToProperty.h"
#include "AruFunctionLibrary.h"

void FAruPredicate_PathToProperty::Execute(FProperty* InProperty, void* InValue) const
{
	if(PathToProperty.IsEmpty() || InValue == nullptr || !Predicate.IsValid())
	{
		return;
	}
	
	FAruPropertyContext PropertyContext = UAruFunctionLibrary::FindPropertyByPath(InProperty, InValue, PathToProperty);
	if(!PropertyContext.IsValid())
	{
		return;
	}
	
	if(const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>())
	{
		PredicatePtr->Execute(PropertyContext.PropertyPtr, PropertyContext.ValuePtr.GetValue());
	}
}
