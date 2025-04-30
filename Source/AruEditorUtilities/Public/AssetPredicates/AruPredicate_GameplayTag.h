#pragma once
#include "AruTypes.h"
#include "GameplayTagContainer.h"
#include "AruPredicate_GameplayTag.generated.h"

USTRUCT(BlueprintType, DisplayName="Set GameplayTag")
struct FAruPredicate_SetGameplayTag : public  FAruPredicate
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FGameplayTag NewVal;
public:
	virtual ~FAruPredicate_SetGameplayTag() override {};
	virtual void Execute(FProperty* InProperty, void* InContainer, void* InValue) const override;
};

USTRUCT(BlueprintType, DisplayName="Set GameplayTag Container")
struct FAruPredicate_SetGameplayTagContainer : public  FAruPredicate
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FGameplayTagContainer NewVal;
public:
	virtual ~FAruPredicate_SetGameplayTagContainer() override {};
	virtual void Execute(FProperty* InProperty, void* InContainer, void* InValue) const override;
};