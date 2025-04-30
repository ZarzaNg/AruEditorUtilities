#include "AssetFilters/AruFilter_ByValue.h"

bool FAruFilter_ByNumericValue::IsConditionMet(FProperty* InProperty, void* InContainer, void* InValue) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return bInverseCondition;
	}

	const FNumericProperty* NumericProperty = CastField<FNumericProperty>(InProperty);
	if(NumericProperty == nullptr)
	{
		return bInverseCondition;
	}

	auto CompareValue = [&](const float NumericValue)
	{
		if(CompareOp == EAruNumericCompareOp::Equip && FMath::IsNearlyEqual(NumericValue, ConditionValue))
		{
			return true;
		}
		if(CompareOp == EAruNumericCompareOp::NotEqual && NumericValue != ConditionValue)
		{
			return true;
		}
		if(CompareOp == EAruNumericCompareOp::GreaterThan && NumericValue > ConditionValue)
		{
			return true;
		}
		if(CompareOp == EAruNumericCompareOp::LessThan && NumericValue < ConditionValue)
		{
			return true;
		}
		return false;
	};

	if(NumericProperty->IsFloatingPoint())
	{
		const float InFloatValue = NumericProperty->GetFloatingPointPropertyValue(InValue);
		return CompareValue(InFloatValue) ^ bInverseCondition;
	}

	if(NumericProperty->IsInteger())
	{
		const int InIntegerValue = NumericProperty->GetSignedIntPropertyValue(InValue);
		return CompareValue(InIntegerValue) ^ bInverseCondition;
	}

	return bInverseCondition;
}

bool FAruFilter_ByBoolean::IsConditionMet(FProperty* InProperty, void* InContainer, void* InValue) const
{
	if(InProperty == nullptr || InValue == nullptr)
	{
		return bInverseCondition;
	}

	const FBoolProperty* BooleanProperty = CastField<FBoolProperty>(InProperty);
	if(BooleanProperty == nullptr)
	{
		return bInverseCondition;
	}

	auto CompareValue = [&](const bool BooleanValue)
	{
		if(CompareOp == EAruBooleanCompareOp::Is && BooleanValue == ConditionValue)
		{
			return true;
		}
		if(CompareOp == EAruBooleanCompareOp::Not && BooleanValue != ConditionValue)
		{
			return true;
		}
		return false;
	};

	return CompareValue(BooleanProperty->GetPropertyValue(InValue)) ^ bInverseCondition;
}

bool FAruFilter_ByObject::IsConditionMet(FProperty* InProperty, void* InContainer, void* InValue) const
{
	FObjectProperty* ObjectProperty = CastField<FObjectProperty>(InProperty);
	if(ObjectProperty == nullptr)
	{
		return bInverseCondition;
	}

	UObject* ObjectPtr = ObjectProperty->GetObjectPropertyValue(InValue);
	
	return (ObjectPtr == ConditionValue) ^ bInverseCondition;
}

bool FAruFilter_ByGameplayTagContainer::IsConditionMet(FProperty* InProperty, void* InContainer, void* InValue) const
{
	if(InProperty == nullptr || InValue == nullptr || TagQuery.IsEmpty())
	{
		return bInverseCondition;
	}

	FStructProperty* StructProperty = CastField<FStructProperty>(InProperty);
	if(StructProperty == nullptr)
	{
		return bInverseCondition;
	}

	const UScriptStruct* StructType = StructProperty->Struct;  
	if(StructType == nullptr)  
	{          
		return bInverseCondition;  
	}

	if(StructType == FGameplayTag::StaticStruct())
	{
		FGameplayTag* GameplayTagPtr = static_cast<FGameplayTag*>(InValue);
		if(GameplayTagPtr == nullptr)
		{
			return bInverseCondition;
		}

		return TagQuery.Matches(FGameplayTagContainer{*GameplayTagPtr}) ^ bInverseCondition;
	}

	if(StructType == FGameplayTagContainer::StaticStruct())
	{
		FGameplayTagContainer* GameplayTagsPtr = static_cast<FGameplayTagContainer*>(InValue);
		if(GameplayTagsPtr == nullptr)
		{
			return bInverseCondition;
		}

		return TagQuery.Matches(*GameplayTagsPtr) ^ bInverseCondition;
	}

	return bInverseCondition;
}
