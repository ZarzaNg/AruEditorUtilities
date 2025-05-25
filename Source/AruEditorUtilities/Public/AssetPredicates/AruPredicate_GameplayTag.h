#pragma once
#include "GameplayTagContainer.h"
#include "AruPredicate_PropertySetter.h"
#include "AruPredicate_GameplayTag.generated.h"

USTRUCT(BlueprintType, DisplayName="Set GameplayTag")
struct FAruPredicate_SetGameplayTag : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	FGameplayTag NewValue;

public:
	virtual ~FAruPredicate_SetGameplayTag() override {}
	virtual const UScriptStruct* GetScriptedStruct() const override { return StaticStruct(); }

	virtual bool Execute(
		const FProperty* InProperty,
		void* InValue,
		const FInstancedPropertyBag& InParameters) const override;
};

USTRUCT(BlueprintType, DisplayName="Set GameplayTag Container")
struct FAruPredicate_SetGameplayTagContainer : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	FGameplayTagContainer NewValue;

public:
	virtual ~FAruPredicate_SetGameplayTagContainer() override {}
	virtual const UScriptStruct* GetScriptedStruct() const override { return StaticStruct(); }

	virtual bool Execute(
		const FProperty* InProperty,
		void* InValue,
		const FInstancedPropertyBag& InParameters) const override;
};
