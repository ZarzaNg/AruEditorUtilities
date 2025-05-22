#pragma once
#include "AruTypes.h"
#include "AruFunctionLibrary.h"
#include "StructUtils/PropertyBag.h"
#include "UObject/PropertyAccessUtil.h"
#include "AruPredicate_PropertySetter.generated.h"

#define LOCTEXT_NAMESPACE "FAruEditorUtilitiesModule"

UENUM(BlueprintType)
enum class EAruValueSource : uint8
{
	Value		UMETA(DisplayName="From Value"),
	Object		UMETA(DisplayName="From Object"),
	DataTable	UMETA(DisplayName="From DataTable"),
	Parameters	UMETA(DisplayName="From Parameters")
};

USTRUCT(meta=(Hidden))
struct FAruPredicate_PropertySetter : public FAruPredicate
{
	GENERATED_BODY()
public:
	virtual ~FAruPredicate_PropertySetter() override {};
	virtual const UScriptStruct* GetScriptedStruct() const {return StaticStruct();}
protected:
	UPROPERTY(EditDefaultsOnly, meta=(DisplayPriority = 0))
	EAruValueSource ValueSource = EAruValueSource::Value;

	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource!=EAruValueSource::Value&&ValueSource!=EAruValueSource::Parameters", EditConditionHides))
	FString PathToProperty{"Path.To.Property"};

	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Object", EditConditionHides))
	TObjectPtr<UObject> Object = nullptr;

	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::DataTable", EditConditionHides))
	FString RowName{};

	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::DataTable", EditConditionHides))
	TObjectPtr<UDataTable> DataTable = nullptr;

	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Parameters", EditConditionHides))
	FString ParameterName{};

protected:
	template<typename T, typename = std::enable_if_t<std::is_base_of_v<FProperty, std::decay_t<T>>>>
	TOptional<const void*> GetNewValueBySourceType(const FInstancedPropertyBag& InParameters, const UStruct* TypeToCheck = nullptr) const
	{
		switch (ValueSource)
		{
		case EAruValueSource::Value:		return GetValueFromStructProperty(T::StaticClass(), TypeToCheck);
		case EAruValueSource::Object:		return GetValueFromObjectAsset(T::StaticClass(), TypeToCheck);
		case EAruValueSource::DataTable:	return GetValueFromDataTable(T::StaticClass(), InParameters, TypeToCheck);
		case EAruValueSource::Parameters:	return {};
		}
		return {};
	}
	
	template<typename T, typename = std::enable_if_t<std::is_base_of_v<FProperty, std::decay_t<T>>>>
	bool SetPropertyValue(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const
	{
		if(InProperty == nullptr || InValue == nullptr)
		{
			return false;
		}

		const T* SubProperty = CastField<T>(InProperty);
		if(SubProperty == nullptr)
		{
			return false;
		}

		TOptional<const void*> OptionalValue = GetNewValueBySourceType<T>(InParameters);
		if(!OptionalValue.IsSet())
		{
			return false;
		}

		const void* PendingValue = OptionalValue.GetValue();
		if(PendingValue == nullptr)
		{
			return false;
		}
	
		SubProperty->CopyCompleteValue(InValue, PendingValue);
		return true;
	}
private:
	TOptional<const void*> GetValueFromStructProperty(const FFieldClass* SourceProperty, const UStruct* SourceType = nullptr) const;
	TOptional<const void*> GetValueFromObjectAsset(const FFieldClass* SourceProperty, const UStruct* SourceType = nullptr) const;
	TOptional<const void*> GetValueFromDataTable(const FFieldClass* SourceProperty, const FInstancedPropertyBag& InParameters, const UStruct* SourceType = nullptr) const;
	
	static bool IsCompatibleType(
		const FProperty* TargetProperty,
		const void* TargetValue,
		const UStruct* SourceType = nullptr);

	static bool IsCompatibleObjectType(
		const FObjectPropertyBase* TargetProperty,
		const void* TargetValue,
		const UStruct* SourceType = nullptr);
	
	static bool IsCompatibleStructType(
		const FStructProperty* TargetProperty,
		const void* TargetValue,
		const UStruct* SourceType = nullptr);
};

USTRUCT(BlueprintType, DisplayName="Set Bool Value")
struct FAruPredicate_SetBoolValue : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	bool NewValue = false;
public:
	virtual ~FAruPredicate_SetBoolValue() override {}
	virtual const UScriptStruct* GetScriptedStruct() const override {return StaticStruct();}
	virtual bool Execute(const FProperty* InProperty, void* InValue,
	                     const FInstancedPropertyBag& InParameters) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Float Value")
struct FAruPredicate_SetFloatValue : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	float NewValue = 0.f;
public:
	virtual ~FAruPredicate_SetFloatValue() override {}
	virtual const UScriptStruct* GetScriptedStruct() const override {return StaticStruct();}
	virtual bool Execute(const FProperty* InProperty, void* InValue,
	                     const FInstancedPropertyBag& InParameters) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Integer Value")
struct FAruPredicate_SetIntegerValue : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	int64 NewValue = 0;
public:
	virtual ~FAruPredicate_SetIntegerValue() override {}
	virtual const UScriptStruct* GetScriptedStruct() const override {return StaticStruct();}
	virtual bool Execute(const FProperty* InProperty, void* InValue,
	                     const FInstancedPropertyBag& InParameters) const override;
};

USTRUCT(BlueprintType, DisplayName="Set String Value")
struct FAruPredicate_SetStringValue : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	FString NewValue{};
public:
	virtual ~FAruPredicate_SetStringValue() override {}
	virtual const UScriptStruct* GetScriptedStruct() const override {return StaticStruct();}
	virtual bool Execute(const FProperty* InProperty, void* InValue,
	                     const FInstancedPropertyBag& InParameters) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Text Value")
struct FAruPredicate_SetTextValue : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	FString NewValue{};
public:
	virtual ~FAruPredicate_SetTextValue() override {}
	virtual const UScriptStruct* GetScriptedStruct() const override {return StaticStruct();}
	virtual bool Execute(const FProperty* InProperty, void* InValue,
	                     const FInstancedPropertyBag& InParameters) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Name Value")
struct FAruPredicate_SetNameValue : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	FName NewValue = FName{};
public:
	virtual ~FAruPredicate_SetNameValue() override {}
	virtual const UScriptStruct* GetScriptedStruct() const override {return StaticStruct();}
	virtual bool Execute(const FProperty* InProperty, void* InValue,
	                     const FInstancedPropertyBag& InParameters) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Enum Value")
struct FAruPredicate_SetEnumValue : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()
protected:

	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	FString NewValue{};
public:
	virtual ~FAruPredicate_SetEnumValue() override{}
	virtual const UScriptStruct* GetScriptedStruct() const override {return StaticStruct();}
	virtual bool Execute(const FProperty* InProperty, void* InValue,
	                     const FInstancedPropertyBag& InParameters) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Struct Value")
struct FAruPredicate_SetStructValue : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	FInstancedStruct NewValue;
public:
	virtual ~FAruPredicate_SetStructValue() override {}
	virtual const UScriptStruct* GetScriptedStruct() const override {return StaticStruct();}
	virtual bool Execute(const FProperty* InProperty, void* InValue,
	                     const FInstancedPropertyBag& InParameters) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Object Value")
struct FAruPredicate_SetObjectValue : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	TObjectPtr<UObject> NewValue = nullptr;
public:
	virtual ~FAruPredicate_SetObjectValue() override {}
	virtual const UScriptStruct* GetScriptedStruct() const override {return StaticStruct();}
	virtual bool Execute(const FProperty* InProperty, void* InValue,
	                     const FInstancedPropertyBag& InParameters) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Instanced Struct Value")
struct FAruPredicate_SetInstancedStructValue : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	FInstancedStruct NewValue;
public:
	virtual ~FAruPredicate_SetInstancedStructValue() override {}
	virtual const UScriptStruct* GetScriptedStruct() const override {return StaticStruct();}
	virtual bool Execute(const FProperty* InProperty, void* InValue,
	                     const FInstancedPropertyBag& InParameters) const override;
};

#undef LOCTEXT_NAMESPACE