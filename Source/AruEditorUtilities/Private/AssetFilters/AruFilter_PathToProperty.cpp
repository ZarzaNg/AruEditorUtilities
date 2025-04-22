#include "AssetFilters/AruFilter_PathToProperty.h"

#include "AruFunctionLibrary.h"

bool FAruFilter_PathToProperty::IsConditionMet(FProperty* InProperty, void* InContainer, void* InValue)
{
	if(PathToProperty.IsEmpty() || InValue == nullptr || !Filter.IsValid())
	{
		return bInverseCondition;
	}

	auto Foo = [&](const FAruPropertyContext& InPropertyContext)
	{
		if(FAruFilter* FilterPtr = Filter.GetMutablePtr<FAruFilter>())
		{
			return FilterPtr->IsConditionMet(InPropertyContext.PropertyPtr, nullptr, InPropertyContext.ValuePtr);
		}

		return false;
	};

	if(const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(InProperty))
	{
		FAruPropertyContext PropertyContext = UAruFunctionLibrary::FindPropertyByPath(ObjectProperty->PropertyClass, InValue, PathToProperty);
		return Foo(PropertyContext);
	}
	
	if(const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty))
	{
		FAruPropertyContext PropertyContext = UAruFunctionLibrary::FindPropertyByPath(StructProperty->Struct, InValue, PathToProperty);
		return Foo(PropertyContext);
	}
	return bInverseCondition;
}
