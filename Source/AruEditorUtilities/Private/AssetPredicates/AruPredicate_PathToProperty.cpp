#include "AssetPredicates/AruPredicate_PathToProperty.h"
#include "AruFunctionLibrary.h"

void FAruPredicate_PathToProperty::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
{
	if(PathToProperty.IsEmpty() || InValue == nullptr || !Predicate.IsValid())
	{
		return;
	}

	auto Foo = [&](const FAruPropertyContext& InPropertyContext)
	{
		if(const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>())
		{
			PredicatePtr->Execute(InPropertyContext.PropertyPtr, nullptr, InPropertyContext.ValuePtr);
		}
	};

	if(const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(InProperty))
	{
		FAruPropertyContext PropertyContext = UAruFunctionLibrary::FindPropertyByPath(ObjectProperty->PropertyClass, InValue, PathToProperty);
		Foo(PropertyContext);
	}
	else if(const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty))
	{
		FAruPropertyContext PropertyContext = UAruFunctionLibrary::FindPropertyByPath(StructProperty->Struct, InValue, PathToProperty);
		Foo(PropertyContext);
	}
}
