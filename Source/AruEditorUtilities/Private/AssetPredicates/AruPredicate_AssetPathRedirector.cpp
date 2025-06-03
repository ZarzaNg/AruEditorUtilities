#include "AssetPredicates/AruPredicate_AssetPathRedirector.h"
#include "AruFunctionLibrary.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AruPredicate_AssetPathRedirector)

#define LOCTEXT_NAMESPACE "AruPredicate_AssetPathRedirector"

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
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"PropertyTypeMismatch",
					"[{0}][{1}]Property:'{1}' is not an uobject."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
		return false;
	}

	UObject* ObjectPtr = ObjectProperty->GetObjectPropertyValue(InValue);
	if (ObjectPtr == nullptr)
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"PropertyValueNull",
					"[{0}][{1}]Property:'{2}' is NULL."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(ObjectProperty->GetName())
			)
		);
		return false;
	}

	TMap<FString, FString> ResolvedReplacementMap;
	Algo::Transform(
		ReplacementMap, ResolvedReplacementMap, [&InParameters](const TTuple<FString, FString>& InTuple)
		{
			return TTuple<FString, FString>
			{
				UAruFunctionLibrary::ResolveParameterizedString(InParameters, InTuple.Key),
				UAruFunctionLibrary::ResolveParameterizedString(InParameters, InTuple.Value)
			};
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

		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
			FText::Format(
				LOCTEXT(
					"OperationSucceed",
					"[{0}][{1}]Previous asset:'{2}', New asset:'{3}' form '{4}'"),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Success),
				FText::FromString(ObjectPtr->GetName()),
				FText::FromString(LoadedAsset->GetName()),
				FText::FromString(NewPath)
			)
		);
		return true;
	}

	return false;
}
#undef LOCTEXT_NAMESPACE
