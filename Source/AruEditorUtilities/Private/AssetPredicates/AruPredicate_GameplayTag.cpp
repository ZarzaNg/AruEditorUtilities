#include "AssetPredicates/AruPredicate_GameplayTag.h"

void FAruPredicate_SetGameplayTag::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
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

	if(auto* PendingValue = GetNewValueBySourceType<FAruPredicate_SetGameplayTag>())
	{
		StructProperty->CopyCompleteValue(InValue, PendingValue);
	}
}

void FAruPredicate_SetGameplayTagContainer::Execute(FProperty* InProperty, void* InContainer, void* InValue) const
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
	
	if(auto* PendingValue = GetNewValueBySourceType<FAruPredicate_SetGameplayTagContainer>())
	{
		StructProperty->CopyCompleteValue(InValue, PendingValue);
	}
}