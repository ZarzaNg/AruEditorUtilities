#include "AssetFilters/AruFilter_ByName.h"
#include "AruFunctionLibrary.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AruFilter_ByName)

bool FAruFilter_ByName::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr)
	{
		return bInverseCondition;
	}

	const FString&& ResolvedPropertyName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, PropertyName);
	if (CompareOp == EAruNameCompareOp::MatchAll)
	{
		return (ResolvedPropertyName == InProperty->GetName()
			 || ResolvedPropertyName == InProperty->GetDisplayNameText().ToString()) ^ bInverseCondition;
	}
	else
	{
		return (InProperty->GetName().Contains(ResolvedPropertyName)
			 || InProperty->GetDisplayNameText().ToString().Contains(ResolvedPropertyName)) ^ bInverseCondition;
	}
}

bool FAruFilter_ByObjectName::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr || InValue == nullptr)
	{
		return bInverseCondition;
	}

	const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(InProperty);
	if (ObjectProperty == nullptr)
	{
		return bInverseCondition;
	}

	UObject* ObjectPtr = ObjectProperty->GetObjectPropertyValue(InValue);
	if (ObjectPtr == nullptr)
	{
		return bInverseCondition;
	}

	const FString&& ResolvedObjectName = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ObjectName);
	if (CompareOp == EAruNameCompareOp::MatchAll)
	{
		return (ResolvedObjectName == ObjectPtr->GetName()) ^ bInverseCondition;
	}
	else
	{
		return (ObjectPtr->GetName().Contains(ResolvedObjectName)) ^ bInverseCondition;
	}
}
