#include "AssetFilters/AruFilter_ByPath.h"
#include "AruFunctionLibrary.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AruFilter_ByPath)

#define LOCTEXT_NAMESPACE "FAruEditorUtilitiesModule"

bool FAruFilter_ByAssetPath::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr || InValue == nullptr)
	{
		return false;
	}

	const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(InProperty);
	if (ObjectProperty == nullptr)
	{
		return false;
	}

	UObject* ObjectPtr = ObjectProperty->GetObjectPropertyValue(InValue);
	if (ObjectPtr == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"Failed to filter by object path",
					"Failed to filter by object path. Property:{0}'s value is nullptr."),
					FText::FromString(ObjectProperty->GetName())
				)
			);
		return false;
	}

	TArray<FString> ResolvedMatchingContexts;
	Algo::Transform(
		MatchingContexts, ResolvedMatchingContexts, [&InParameters](const FString& InStringValue)
		{
			return UAruFunctionLibrary::ResolveParameterizedString(InParameters, InStringValue);
		});

	const FString AssetPath = ObjectPtr->GetPathName();
	if (AssetPath.IsEmpty())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"Failed to filter by object path",
					"Failed to filter by object:'{0}''s path. Asset's path is empty."),
					FText::FromString(ObjectPtr->GetName())
				)
			);
		return false;
	}

	bool Result = ResolvedMatchingContexts.Num() > 0;
	for (auto& Context : ResolvedMatchingContexts)
	{
		Result &= AssetPath.Contains(Context);
	}

	if (Result == true)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
			FText::Format(
				LOCTEXT(
					"Filtering by object path",
					"Asset path:'{0}' matched."),
				FText::FromString(AssetPath)
			)
		);
	}
	else
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
			FText::Format(
				LOCTEXT(
					"Filtering by object path",
					"Asset path:'{0}' do not match."),
				FText::FromString(AssetPath)
			)
		);
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
