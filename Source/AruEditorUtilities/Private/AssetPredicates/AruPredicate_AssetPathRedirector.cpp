#include "AssetPredicates/AruPredicate_AssetPathRedirector.h"

bool FAruPredicate_AssetPathRedirector::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
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

	TMap<FString, FString> ResolvedReplacementMap;
	Algo::Transform(
		ReplacementMap, ResolvedReplacementMap, [&InParameters](const TTuple<FString, FString>& InTuple)
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

			return TTuple<FString, FString>{ResolveParameterizedString(InTuple.Key), ResolveParameterizedString(InTuple.Value)};
		});

	TArray<FString> SortedKeys;
	ResolvedReplacementMap.GetKeys(SortedKeys);
	SortedKeys.Sort(
		[](const FString& A, const FString& B)
		{
			return A.Len() > B.Len();
		});

	TArray<FString> PathSegments;
	ObjectPtr->GetPathName().ParseIntoArray(PathSegments, TEXT("/"), true);
	for (FString& Segment : PathSegments)
	{
		FString ModifiedSegment = Segment;
		for (const FString& Key : SortedKeys)
		{
			if (!ModifiedSegment.Contains(Key))
			{
				continue;
			}

			if (const FString* Replacement = ResolvedReplacementMap.Find(Key))
			{
				ModifiedSegment = ModifiedSegment.Replace(*Key, **Replacement, ESearchCase::CaseSensitive);
			}
		}
		Segment = ModifiedSegment;
	}

	FString NewPath = FString::Printf(TEXT("/%s"), *FString::Join(PathSegments, TEXT("/")));
	const FSoftObjectPath TargetAssetPath{NewPath};
	if (UObject* LoadedAsset = TargetAssetPath.TryLoad())
	{
		ObjectProperty->SetObjectPropertyValue(InValue, LoadedAsset);
		return true;
	}

	return false;
}
