#pragma once
#include "AruTypes.h"
#include "AruPredicate_PropertySetter.generated.h"

USTRUCT(BlueprintType, DisplayName="Set Bool Value")
struct FAruPredicate_SetBoolValue : public FAruPredicate
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	bool NewVal = false;
public:
	virtual ~FAruPredicate_SetBoolValue() override {};
	virtual void Execute(FProperty* InProperty, void* InContainer, void* InValue) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Float Value")
struct FAruPredicate_SetFloatValue : public FAruPredicate
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	float NewVal = 0.f;
public:
	virtual ~FAruPredicate_SetFloatValue() override {};
	virtual void Execute(FProperty* InProperty, void* InContainer, void* InValue) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Integer Value")
struct FAruPredicate_SetIntegerValue : public FAruPredicate
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	int64 NewVal = 0;
public:
	virtual ~FAruPredicate_SetIntegerValue() override {};
	virtual void Execute(FProperty* InProperty, void* InContainer, void* InValue) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Name Value")
struct FAruPredicate_SetNameValue : public FAruPredicate
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FName NewVal = FName{};
public:
	virtual ~FAruPredicate_SetNameValue() override {};
	virtual void Execute(FProperty* InProperty, void* InContainer, void* InValue) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Struct Value")
struct FAruPredicate_SetStructValue : public FAruPredicate
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FInstancedStruct NewVal;
public:
	virtual ~FAruPredicate_SetStructValue() override {};
	virtual void Execute(FProperty* InProperty, void* InContainer, void* InValue) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Object Value")
struct FAruPredicate_SetObjectValue : public FAruPredicate
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	TObjectPtr<UObject> NewVal = nullptr;
public:
	virtual ~FAruPredicate_SetObjectValue() override {};
	virtual void Execute(FProperty* InProperty, void* InContainer, void* InValue) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Instanced Struct Value")
struct FAruPredicate_SetInstancedStructValue : public FAruPredicate
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, SimpleDisplay)
	FInstancedStruct NewVal;
public:
	virtual ~FAruPredicate_SetInstancedStructValue() override {};
	virtual void Execute(FProperty* InProperty, void* InContainer, void* InValue) const override;
};
