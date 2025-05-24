#include "AssetFilters/AruFilter_ByPath.h"

bool FAruFilter_ByObjectPath::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
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
		return false;
	}

	TArray<FString> ResolvedMatchingContexts;
	Algo::Transform(
		MatchingContexts, ResolvedMatchingContexts, [&InParameters](const FString& InStringValue)
		{
			auto ResolveParameterizedString = [&InParameters](const FString& SourceString)
			{
				if (SourceString.IsEmpty() || SourceString[0] != '@')
				{
					return SourceString;
				}

				TValueOrError<FString, EPropertyBagResult> SearchStringResult = InParameters.GetValueString(FName{SourceString.RightChop(1)});
				if (SearchStringResult.HasValue())
				{
					return SearchStringResult.GetValue();
				}

				return SourceString;
			};

			return ResolveParameterizedString(InStringValue);
		});
	
	const FString AssetPath = ObjectPtr->GetPathName();
	if(AssetPath.IsEmpty())
	{
		return false;
	}

	bool Result = ResolvedMatchingContexts.Num() > 0;
	for(auto& Context : ResolvedMatchingContexts)
	{
		Result &= AssetPath.Contains(Context);
	}

	return Result;
}
