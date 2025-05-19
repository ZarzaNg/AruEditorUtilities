#include "AssetPredicates/AruPredicate_GameplayTag.h"

void FAruPredicate_SetGameplayTag::Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}

	const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty);
	if(StructProperty == nullptr)
	{
		return;
	}

	const UScriptStruct* SourceStructType = StructProperty->Struct;
	if(SourceStructType == nullptr || SourceStructType != FGameplayTag::StaticStruct())  
	{          
		return;  
	}

	FGameplayTag* GameplayTagPtr = static_cast<FGameplayTag*>(InValue);
	if(GameplayTagPtr == nullptr)
	{
		return;
	}

	if(auto* PendingValue = GetNewValueBySourceType<FStructProperty>(InParameters).GetPtrOrNull())
	{
		StructProperty->CopyCompleteValue(InValue, *PendingValue);
	}
}

void FAruPredicate_SetGameplayTagContainer::Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return;
	}

	const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty);
	if(StructProperty == nullptr)
	{
		return;
	}

	const UScriptStruct* SourceStructType = StructProperty->Struct;
	if(SourceStructType == nullptr || SourceStructType != FGameplayTagContainer::StaticStruct())  
	{          
		return;  
	}

	FGameplayTagContainer* GameplayTagPtr = static_cast<FGameplayTagContainer*>(InValue);
	if(GameplayTagPtr == nullptr)
	{
		return;
	}
	
	if(auto* PendingValue = GetNewValueBySourceType<FStructProperty>(InParameters).GetPtrOrNull())
	{
		StructProperty->CopyCompleteValue(InValue, *PendingValue);
	}
}