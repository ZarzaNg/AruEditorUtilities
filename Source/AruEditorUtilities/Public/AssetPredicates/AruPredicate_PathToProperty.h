#pragma once
#include "AruTypes.h"
#include "AruPredicate_PathToProperty.generated.h"

USTRUCT(BlueprintType, DisplayName="Path To Property")
struct FAruPredicate_PathToProperty : public FAruPredicate
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FString PathToProperty{"Path.To.Your.Property"};

	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TInstancedStruct<FAruPredicate> Predicate;
public:
	virtual ~FAruPredicate_PathToProperty() override {};
	virtual void Execute(FProperty* InProperty, void* InContainer, void* InValue) const override;
};