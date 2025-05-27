#include "AssetPredicates/AruPredicate_GameplayTag.h"
#define LOCTEXT_NAMESPACE "FAruEditorUtilitiesModule"

bool FAruPredicate_SetGameplayTag::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr || InValue == nullptr)
	{
		return false;
	}

	const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty);
	if (StructProperty == nullptr)
	{
		return false;
	}

	const UScriptStruct* SourceStructType = StructProperty->Struct;
	if (SourceStructType == nullptr || SourceStructType != FGameplayTag::StaticStruct())
	{
		return false;
	}

	FGameplayTag* GameplayTagPtr = static_cast<FGameplayTag*>(InValue);
	if (GameplayTagPtr == nullptr)
	{
		return false;
	}

	if (auto* PendingValue = GetNewValueBySourceType<FStructProperty>(InParameters).GetPtrOrNull())
	{
		StructProperty->CopyCompleteValue(InValue, *PendingValue);
		return true;
	}

	return false;
}

bool FAruPredicate_SetGameplayTagContainer::Execute(
	const FProperty* InProperty,
	void* InValue,
	const FInstancedPropertyBag& InParameters) const
{
	if (InProperty == nullptr || InValue == nullptr)
	{
		return false;
	}

	const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty);
	if (StructProperty == nullptr)
	{
		return false;
	}

	const UScriptStruct* SourceStructType = StructProperty->Struct;
	if (SourceStructType == nullptr || SourceStructType != FGameplayTagContainer::StaticStruct())
	{
		return false;
	}

	FGameplayTagContainer* GameplayTagPtr = static_cast<FGameplayTagContainer*>(InValue);
	if (GameplayTagPtr == nullptr)
	{
		return false;
	}

	if (auto* PendingValue = GetNewValueBySourceType<FStructProperty>(InParameters).GetPtrOrNull())
	{
		StructProperty->CopyCompleteValue(InValue, *PendingValue);
		return true;
	}

	return false;
}
#undef LOCTEXT_NAMESPACE