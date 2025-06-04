#pragma once
#include "AruTypes.h"
#include "AruPredicate_PathToProperty.generated.h"

USTRUCT(BlueprintType, DisplayName="Path To Property")
struct FAruPredicate_PathToProperty : public FAruPredicate
{
	GENERATED_BODY()
public:
	virtual ~FAruPredicate_PathToProperty() override {};

	virtual bool Execute(
		const FProperty* InProperty,
		void* InValue,
		const FInstancedPropertyBag& InParameters) const override;

protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FString PathToProperty{"Path.To.Your.Property"};

	UPROPERTY(EditDefaultsOnly, meta=(ExcludeBaseStruct))
	TInstancedStruct<FAruPredicate> Predicate;

private:
	static FString GetCompactName() { return {"FindPropertyByPath"}; }
};
