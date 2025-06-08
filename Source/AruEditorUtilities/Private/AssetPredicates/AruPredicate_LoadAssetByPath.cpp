#include "AssetPredicates/AruPredicate_LoadAssetByPath.h"

#include "AruFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AruPredicate_LoadAssetByPath)

#define LOCTEXT_NAMESPACE "AruPredicate_LoadAssetByPath"

bool FAruPredicate_LoadAssetByPath::Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr || InValue == nullptr)
	{
		return false;
	}

	if (PathToAsset.IsEmpty())
	{
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
			FText::Format(
				LOCTEXT(
					"ObjectPathEmpty",
					"[{0}][{1}]Property:'{1}'. Asset path is empty."),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Failed),
				FText::FromString(InProperty->GetName())
			));
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

	TArray<FString> PathSegments;
	PathToAsset.ParseIntoArray(PathSegments, TEXT("/"), true);
	for (auto& Segment : PathSegments)
	{
		Segment = UAruFunctionLibrary::ResolveParameterizedString(InParameters, Segment);
	}

	const FString ResolvedPath = FString::Printf(TEXT("/%s"), *FString::Join(PathSegments, TEXT("/")));
	const FSoftObjectPath TargetAssetPath{ResolvedPath};
	if (UObject* LoadedAsset = TargetAssetPath.TryLoad())
	{
		if (!LoadedAsset->IsA(ObjectProperty->PropertyClass))
		{
			FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
				FText::Format(
					LOCTEXT(
						"ClassTypeMismatch",
						"[{0}][{1}]Property:'{2}' class:{3}, new object class:{4}."),
					FText::FromString(GetCompactName()),
					FText::FromString(Aru::ProcessResult::Failed),
					FText::FromString(InProperty->GetName()),
					FText::FromString(ObjectProperty->PropertyClass ? ObjectProperty->PropertyClass->GetName() : FString{"NULL"}),
					FText::FromString(LoadedAsset->GetClass() ? LoadedAsset->GetClass()->GetName() : FString{"NULL"})
				));

			return false;
		}

		ObjectProperty->SetObjectPropertyValue(InValue, LoadedAsset);
		FMessageLog{FName{"AruEditorUtilitiesModule"}}.Info(
			FText::Format(
				LOCTEXT(
					"OperationSucceed",
					"[{0}][{1}]New asset:'{3}' from '{4}'"),
				FText::FromString(GetCompactName()),
				FText::FromString(Aru::ProcessResult::Success),
				FText::FromString(LoadedAsset->GetName()),
				FText::FromString(ResolvedPath)
			)
		);
		return true;
	}

	FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
		FText::Format(
			LOCTEXT(
				"ObjectNotFound",
				"[{0}][{1}]Property:'{2}'. Object not found by path:'{3}'."),
			FText::FromString(GetCompactName()),
			FText::FromString(Aru::ProcessResult::Failed),
			FText::FromString(InProperty->GetName()),
			FText::FromString(ResolvedPath)
		));

	return false;
}

#undef LOCTEXT_NAMESPACE
