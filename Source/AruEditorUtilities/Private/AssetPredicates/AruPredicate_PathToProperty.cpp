#include "AssetPredicates/AruPredicate_PathToProperty.h"
#include "AruFunctionLibrary.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AruPredicate_PathToProperty)

#define LOCTEXT_NAMESPACE "AruPredicate_PathToProperty"

bool FAruPredicate_PathToProperty::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (PathToProperty.IsEmpty() || InValue == nullptr || !Predicate.IsValid())
	{
		return false;
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
		return false;
	}

	bool bExecutedSuccessfully = false;
	if (const FAruPredicate* PredicatePtr = Predicate.GetPtr<FAruPredicate>())
	{
		bExecutedSuccessfully |= PredicatePtr->Execute(PropertyContext.PropertyPtr, PropertyContext.ValuePtr.GetValue(), InParameters);
	}
	return bExecutedSuccessfully;
}
#undef LOCTEXT_NAMESPACE