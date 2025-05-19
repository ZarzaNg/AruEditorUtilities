#include "AssetPredicates/AruPredicate_AssetPathRedirector.h"

void FAruPredicate_AssetPathRedirector::Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}
	
	const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(InProperty);
	if(ObjectProperty == nullptr)
	{
		return;
	}

	UObject* ObjectPtr = ObjectProperty->GetObjectPropertyValue(InValue);
	if(ObjectPtr == nullptr)
	{
		return;
	}
	
	FString OriginalPath = ObjectPtr->GetPathName();

	TArray<FString> PathSegments;
	OriginalPath.ParseIntoArray(PathSegments, TEXT("/"), true);

	TArray<FString> SortedKeys;
	ReplacementMap.GetKeys(SortedKeys);
	SortedKeys.Sort([](const FString& A, const FString& B) {
		return A.Len() > B.Len(); 
	});
	
	for (FString& Segment : PathSegments)
	{
		FString ModifiedSegment = Segment;
		for (const FString& Key : SortedKeys)
		{
			if (ModifiedSegment.Contains(Key))
			{
				const FString* Replacement = ReplacementMap.Find(Key);
				ModifiedSegment = ModifiedSegment.Replace(*Key, **Replacement, ESearchCase::CaseSensitive);
			}
		}
		Segment = ModifiedSegment;
	}

	FString NewPath = FString::Printf(TEXT("/%s"), *FString::Join(PathSegments, TEXT("/")));

	FSoftObjectPath TargetAssetPath(NewPath);
	if (UObject* LoadedAsset = TargetAssetPath.TryLoad())
	{
		ObjectProperty->SetObjectPropertyValue(InValue, LoadedAsset);
	}
}
