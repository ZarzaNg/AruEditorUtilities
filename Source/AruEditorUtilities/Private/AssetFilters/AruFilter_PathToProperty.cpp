#include "AssetFilters/AruFilter_PathToProperty.h"
#include "AruFunctionLibrary.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AruFilter_PathToProperty)

#define LOCTEXT_NAMESPACE "FAruEditorUtilitiesModule"

bool FAruFilter_PathToProperty::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if (PathToProperty.IsEmpty() || InValue == nullptr || !Filter.IsValid())
	{
		return bInverseCondition;
	}

	TArray<FString> PropertyChain;
	PathToProperty.ParseIntoArray(PropertyChain, TEXT("."), true);
	for (auto& Element : PropertyChain)
	{
		Element = UAruFunctionLibrary::ResolveParameterizedString(InParameters, Element);
	}
	const FString&& ResolvedPath = FString::Join(PropertyChain, TEXT("."));

	FAruPropertyContext PropertyContext = UAruFunctionLibrary::FindPropertyByPath(InProperty, InValue, ResolvedPath);
	if (!PropertyContext.IsValid())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"No property found.",
					"Failed to find property by path:'{0}'."),
					FText::FromString(ResolvedPath)
				)
			);
		return bInverseCondition;
	}

	if (const FAruFilter* FilterPtr = Filter.GetPtr<FAruFilter>())
	{
		return FilterPtr->IsConditionMet(PropertyContext.PropertyPtr, PropertyContext.ValuePtr.GetValue(), InParameters);
	}

	return bInverseCondition;
}

#undef LOCTEXT_NAMESPACE