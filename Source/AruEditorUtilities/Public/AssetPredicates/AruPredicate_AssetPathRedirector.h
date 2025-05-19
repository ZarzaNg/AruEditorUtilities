#pragma once
#include "AruTypes.h"
#include "AruPredicate_AssetPathRedirector.generated.h"

USTRUCT(BlueprintType, DisplayName="AssetPath Redirector")
struct FAruPredicate_AssetPathRedirector : public FAruPredicate
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TMap<FString, FString> ReplacementMap;
	
	virtual ~FAruPredicate_AssetPathRedirector() override {}
	virtual void Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const override;
};