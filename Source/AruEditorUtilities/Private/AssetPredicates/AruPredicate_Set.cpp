#include "AssetPredicates/AruPredicate_Set.h"

void FAruPredicate_AddSetElement::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
{
	const FSetProperty* SetProperty = CastField<FSetProperty>(InProperty);
	if(SetProperty == nullptr)
	{
		return;
	}

	if(Predicates.Num() == 0)
	{
		return;
	}
	
	FScriptSetHelper SetHelper(SetProperty, InValue);
	int32 NewElementIndex = SetHelper.AddDefaultValue_Invalid_NeedsRehash();

	ON_SCOPE_EXIT
	{
		SetHelper.Rehash();
	};
	
	if(!SetHelper.IsValidIndex(NewElementIndex))
	{
		return;
	}
	
	FProperty* ElementProperty = SetProperty->ElementProp;
	void* NewElementPtr = SetHelper.GetElementPtr(NewElementIndex);
	for(auto& Predicate : Predicates)
	{
		const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>();
		if(PredicatePtr == nullptr)
		{
			continue;
		}
		
		PredicatePtr->Execute(ElementProperty, InContainer, NewElementPtr);
	}
}

void FAruPredicate_RemoveSetValue::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
{
	FAruPredicate::Execute(InProperty, InContainer, InValue);
}

void FAruPredicate_ModifySetValue::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
{
	FAruPredicate::Execute(InProperty, InContainer, InValue);
}
