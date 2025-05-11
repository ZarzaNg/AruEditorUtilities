#pragma once
#include "AruTypes.h"
#include "AruFunctionLibrary.h"
#include "UObject/PropertyAccessUtil.h"
#include "AruPredicate_PropertySetter.generated.h"

#define LOCTEXT_NAMESPACE "FAruEditorUtilitiesModule"

UENUM(BlueprintType)
enum class EAruValueSource : uint8
{
	Value		UMETA(DisplayName="From Value"),
	Object		UMETA(DisplayName="From Object"),
	DataTable	UMETA(DisplayName="From DataTable")
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

	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource!=EAruValueSource::Value", EditConditionHides))
	FString PathToProperty{"Path.To.Property"};

	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Object", EditConditionHides))
	TObjectPtr<UObject> Object = nullptr;

	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::DataTable", EditConditionHides))
	FName RowName = FName{"None"};

	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::DataTable", EditConditionHides))
	TObjectPtr<UDataTable> DataTable = nullptr;

	template<typename T, typename = std::enable_if_t<std::is_base_of_v<FProperty, std::decay_t<T>>>>
	TOptional<const void*> GetNewValueBySourceType(const UStruct* TypeToCheck = nullptr) const
	{
		auto IsCompatibleType = [&TypeToCheck](const FProperty* NewValueType, const void* NewValue) -> bool
		{
			if(!NewValueType->IsA<T>())
			{
				return false;
			}
			
			if(TypeToCheck == nullptr)
			{
				return true;
			}

			if(const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(NewValueType))
			{
				const UObject* Object = ObjectProperty->GetObjectPropertyValue(NewValue);
				if(Object == nullptr)
				{
					// We might want to clear the property value.
					// So when we got nullptr, we consider it as matched.
					return true;
				}
				
				const UClass* ObjectClass = Object->GetClass();
				if(ObjectClass == nullptr)
				{
					return false;
				}

				return ObjectClass->IsChildOf(TypeToCheck);
			}
			
			if(const FStructProperty* StructProperty = CastField<FStructProperty>(NewValueType))
			{
				const UScriptStruct* ScriptStruct = StructProperty->Struct;
				if(ScriptStruct == nullptr)
				{
					return false;
				}

				if(TypeToCheck == FInstancedStruct::StaticStruct())
				{
					return ScriptStruct == FInstancedStruct::StaticStruct();
				}

				const FInstancedStruct* InstancedStructPtr = static_cast<const FInstancedStruct*>(NewValue);  
				if(InstancedStructPtr == nullptr)  
				{             
					return false;  
				}

				const UScriptStruct* InnerStruct = InstancedStructPtr->GetScriptStruct();
				if(InnerStruct == nullptr)
				{
					return false;
				}
				
				return InnerStruct->IsChildOf(TypeToCheck);
			}

			return false;
		};
		
		switch (ValueSource)
		{
		case EAruValueSource::Value:
			{
				static FName ValueName{"NewValue"};
				const FProperty* Property = PropertyAccessUtil::FindPropertyByName(ValueName, GetScriptedStruct());
				if(!ensureMsgf(Property != nullptr, TEXT("Can't find NewValue property in derived struct.")))
				{
					return {};
				}
				
				const void* PropertyValue = Property->ContainerPtrToValuePtr<void>(this);
				if(!IsCompatibleType(Property, PropertyValue))
				{
					FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Type mismatches", "Mismatch between target and source value types."));
					return {};
				}
				
				return TOptional<const void*>{PropertyValue};
			}
		case EAruValueSource::Object:
			{
				if(PathToProperty.IsEmpty() || Object == nullptr)
				{
					FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Find value failed", "Please check 'PathToProperty' and 'Object' configs."));
					return {};
				}

				const UClass* NativeClass = Object.GetClass();
				const UObject* NativeObject = Object;
				if(UBlueprint* BlueprintObject = Cast<UBlueprint>(Object))
				{
					NativeClass = BlueprintObject->GeneratedClass;
					NativeObject= NativeClass->GetDefaultObject();
				}

				auto&& PropertyContext = UAruFunctionLibrary::FindPropertyByPath(NativeClass, NativeObject, PathToProperty);
				if(!PropertyContext.IsValid())
				{
					FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
						FText::Format(LOCTEXT("Find value failed", "Can't find property by path: '{0}' in object: '{1}'."),
							FText::FromString(PathToProperty),
							FText::FromName(Object.GetFName())));
					return {};
				}

				if(!IsCompatibleType(PropertyContext.PropertyPtr, PropertyContext.ValuePtr.GetValue()))
				{
					FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Type mismatches", "Mismatch between target and source value types."));
					return {};
				}

				return TOptional<const void*>{PropertyContext.ValuePtr.GetValue()};
			}
		case EAruValueSource::DataTable:
			{
				if(RowName.IsNone() || PathToProperty.IsEmpty() || DataTable == nullptr)
				{
					FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Find value failed", "Please check 'PathToProperty' and 'Object' as well as 'DataTable' configs."));
					return {};
				}

				uint8* const* RowStructPtr = DataTable->GetRowMap().Find(RowName);
				if(RowStructPtr == nullptr)
				{
					FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
						FText::Format(LOCTEXT("Find value failed", "Can't find row: '{0}' in DataTable: '{1}'."),
							FText::FromName(RowName),
							FText::FromName(DataTable.GetFName())));
					return {};
				}

				uint8* RowStruct = *RowStructPtr;
				if(RowStruct == nullptr)
				{
					return {};
				}

				auto&& PropertyContext = UAruFunctionLibrary::FindPropertyByPath(DataTable->RowStruct, RowStruct, PathToProperty);
				if(!PropertyContext.IsValid())
				{
					FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(
						FText::Format(LOCTEXT("Find value failed", "Can't find property by path: '{0}' in struct: '{1}'."),
							FText::FromString(PathToProperty),
							FText::FromName(DataTable->RowStruct.GetFName())));
					return {};
				}

				if(!IsCompatibleType(PropertyContext.PropertyPtr, PropertyContext.ValuePtr.GetValue()))
				{
					FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Type mismatches", "Mismatch between target and source value types."));
					return {};
				}

				return TOptional<const void*>{PropertyContext.ValuePtr.GetValue()};
			}
		}

		return {};
	}
	
	template<typename T, typename = std::enable_if_t<std::is_base_of_v<FProperty, std::decay_t<T>>>>
	void SetPropertyValue(const FProperty* InProperty, void* InValue) const
	{
		if(InProperty == nullptr || InValue == nullptr)
		{
			return;
		}

		const T* SubProperty = CastField<T>(InProperty);
		if(SubProperty == nullptr)
		{
			return;
		}

		TOptional<const void*> OptionalValue = GetNewValueBySourceType<T>();
		if(!OptionalValue.IsSet())
		{
			return;
		}

		const void* PendingValue = OptionalValue.GetValue();
		if(PendingValue == nullptr)
		{
			return;
		}
	
		SubProperty->CopyCompleteValue(InValue, PendingValue);
	}
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
	virtual void Execute(const FProperty* InProperty, void* InValue) const override;
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
	virtual void Execute(const FProperty* InProperty, void* InValue) const override;
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
	virtual void Execute(const FProperty* InProperty, void* InValue) const override;
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
	virtual void Execute(const FProperty* InProperty, void* InValue) const override;
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
	virtual void Execute(const FProperty* InProperty, void* InValue) const override;
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
	virtual void Execute(const FProperty* InProperty, void* InValue) const override;
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
	virtual void Execute(const FProperty* InProperty, void* InValue) const override;
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
	virtual void Execute(const FProperty* InProperty, void* InValue) const override;
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
	virtual void Execute(const FProperty* InProperty, void* InValue) const override;
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
	virtual void Execute(const FProperty* InProperty, void* InValue) const override;
};

#undef LOCTEXT_NAMESPACE