#include "AssetFilters/AruFilter_ByValue.h"

#include "AruFunctionLibrary.h"

bool FAruFilter_ByNumericValue::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
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

bool FAruFilter_ByBoolean::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
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

bool FAruFilter_ByObject::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
{
	const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(InProperty);
	if(ObjectProperty == nullptr)
	{
		return bInverseCondition;
	}

	UObject* ObjectPtr = ObjectProperty->GetObjectPropertyValue(InValue);
	
	return (ObjectPtr == ConditionValue) ^ bInverseCondition;
}

bool FAruFilter_ByEnum::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || ConditionValue.IsEmpty())
	{
		return bInverseCondition;
	}

	const FEnumProperty* EnumProperty = CastField<FEnumProperty>(InProperty);
	if(EnumProperty == nullptr)
	{
		return bInverseCondition;
	}

	const UEnum* EnumType = EnumProperty->GetEnum();
	if(EnumType == nullptr)
	{
		return bInverseCondition;
	}

	const FNumericProperty* UnderlyingProperty = EnumProperty->GetUnderlyingProperty();
	if(UnderlyingProperty == nullptr)
	{
		return bInverseCondition;
	}
	
	const int64 ConditionEnumValue = EnumType->GetValueByNameString(*ConditionValue);
	if(ConditionEnumValue == INDEX_NONE)
	{
		return bInverseCondition;
	}
	
	const int64 InEnumValue = UnderlyingProperty->GetSignedIntPropertyValue(InValue);
	return	(
			(CompareOp == EAruBooleanCompareOp::Is && ConditionEnumValue == InEnumValue) ||
			(CompareOp == EAruBooleanCompareOp::Not && ConditionEnumValue != InEnumValue)
			) ^ bInverseCondition;
}

bool FAruFilter_ByString::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || ConditionValue.IsEmpty())
	{
		return bInverseCondition;
	}

	const FStrProperty* StrProperty = CastField<FStrProperty>(InProperty);
	if(StrProperty == nullptr)
	{
		return bInverseCondition;
	}

	const FString* InStringValue = static_cast<const FString*>(InValue);
	if(InStringValue == nullptr)
	{
		return bInverseCondition;
	}

	ESearchCase::Type SearchCase = bCaseSensitive? ESearchCase::CaseSensitive : ESearchCase::IgnoreCase;
	const FString&& ResolvedConditionValue = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ConditionValue);
	if(CompareOp == EAruContainerCompareOp::HasAll)
	{
		return InStringValue->Equals(ResolvedConditionValue, SearchCase) ^ bInverseCondition;
	}
	else if(CompareOp == EAruContainerCompareOp::HasAny)
	{
		return InStringValue->Contains(ResolvedConditionValue, SearchCase) ^ bInverseCondition;
	}

	return bInverseCondition;
}

bool FAruFilter_ByText::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || ConditionValue.IsEmpty())
	{
		return bInverseCondition;
	}

	const FTextProperty* TextProperty = CastField<FTextProperty>(InProperty);
	if(TextProperty == nullptr)
	{
		return bInverseCondition;
	}

	const FText* InTextValue = static_cast<const FText*>(InValue);
	if(InTextValue == nullptr)
	{
		return bInverseCondition;
	}

	const FString& InStringValue = InTextValue->ToString();
	ESearchCase::Type SearchCase = bCaseSensitive? ESearchCase::CaseSensitive : ESearchCase::IgnoreCase;
	const FString&& ResolvedConditionValue = UAruFunctionLibrary::ResolveParameterizedString(InParameters, ConditionValue);
	if(CompareOp == EAruContainerCompareOp::HasAll)
	{
		return InStringValue.Equals(ResolvedConditionValue, SearchCase) ^ bInverseCondition;
	}
	else if(CompareOp == EAruContainerCompareOp::HasAny)
	{
		return InStringValue.Contains(ResolvedConditionValue, SearchCase) ^ bInverseCondition;
	}

	return bInverseCondition;
}

bool FAruFilter_ByGameplayTagContainer::IsConditionMet(const FProperty* InProperty, const void* InValue, const FInstancedPropertyBag& InParameters) const
{
	if(InProperty == nullptr || InValue == nullptr || TagQuery.IsEmpty())
	{
		return bInverseCondition;
	}

	const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty);
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
		const FGameplayTag* GameplayTagPtr = static_cast<const FGameplayTag*>(InValue);
		if(GameplayTagPtr == nullptr)
		{
			return bInverseCondition;
		}

		return TagQuery.Matches(FGameplayTagContainer{*GameplayTagPtr}) ^ bInverseCondition;
	}

	if(StructType == FGameplayTagContainer::StaticStruct())
	{
		const FGameplayTagContainer* GameplayTagsPtr = static_cast<const FGameplayTagContainer*>(InValue);
		if(GameplayTagsPtr == nullptr)
		{
			return bInverseCondition;
		}

		return TagQuery.Matches(*GameplayTagsPtr) ^ bInverseCondition;
	}

	return bInverseCondition;
}
