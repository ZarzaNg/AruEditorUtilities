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

USTRUCT()
struct FAruPredicate_PropertySetter : public FAruPredicate
{
	GENERATED_BODY()
public:
	virtual ~FAruPredicate_PropertySetter() override {};
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

	template<typename T>
	TOptional<const void*> GetNewValueBySourceType(const UStruct* TypeToMatch = nullptr) const
	{
		auto TypeMatch = [&TypeToMatch](const FProperty* InProperty, const void* InValue) -> bool
		{
			if(TypeToMatch == nullptr)
			{
				return true;
			}

			if(const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(InProperty))
			{
				const UObject* Object = ObjectProperty->GetObjectPropertyValue(InValue);
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

				return ObjectClass->IsChildOf(TypeToMatch);
			}
			
			if(const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty))
			{
				const UScriptStruct* ScriptStruct = StructProperty->Struct;
				if(ScriptStruct == nullptr)
				{
					return false;
				}

				if(TypeToMatch == FInstancedStruct::StaticStruct())
				{
					return ScriptStruct == FInstancedStruct::StaticStruct();
				}

				const FInstancedStruct* InstancedStructPtr = static_cast<const FInstancedStruct*>(InValue);  
				if(InstancedStructPtr == nullptr)  
				{             
					return false;  
				}

				const UScriptStruct* InnerStruct = InstancedStructPtr->GetScriptStruct();
				if(InnerStruct == nullptr)
				{
					return false;
				}
				
				return InnerStruct->IsChildOf(TypeToMatch);
			}

			return false;
		};

		auto PointerWrapper = [&TypeToMatch](const FProperty* InProperty, const void* InValue) -> const void*
		{
			if(const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty))
			{
				if(TypeToMatch == FInstancedStruct::StaticStruct())
				{
					return InValue;
				}
				
				const UScriptStruct* ScriptStruct = StructProperty->Struct;
				if(ScriptStruct == nullptr)
				{
					return InValue;
				}

				if(ScriptStruct != FInstancedStruct::StaticStruct())
				{
					return InValue;
				}

				const FInstancedStruct* InstancedStructPtr = static_cast<const FInstancedStruct*>(InValue);  
				if(InstancedStructPtr == nullptr)  
				{             
					return InValue;  
				}

				return InstancedStructPtr->GetMemory();
			}

			return InValue;
		};
		
		switch (ValueSource)
		{
		case EAruValueSource::Value:
			{
				static FName ValueName{"NewValue"};
				const FProperty* Property = PropertyAccessUtil::FindPropertyByName(ValueName, T::StaticStruct());
				if(!ensureMsgf(Property != nullptr, TEXT("Can't find NewValue property in derived struct.")))
				{
					return {};
				}
				
				const void* PropertyValue = Property->ContainerPtrToValuePtr<void>(this);
				if(!TypeMatch(Property, PropertyValue))
				{
					FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Type mismatches", "Mismatch between target and source value types."));
					return {};
				}
				
				return TOptional<const void*>(PointerWrapper(Property, PropertyValue));
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

				if(!TypeMatch(PropertyContext.PropertyPtr, PropertyContext.ValuePtr.GetValue()))
				{
					FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Type mismatches", "Mismatch between target and source value types."));
					return {};
				}

				return TOptional<const void*>(PointerWrapper(PropertyContext.PropertyPtr, PropertyContext.ValuePtr.GetValue()));
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

				if(!TypeMatch(PropertyContext.PropertyPtr, PropertyContext.ValuePtr.GetValue()))
				{
					FMessageLog{FName{"AruEditorUtilitiesModule"}}.Warning(LOCTEXT("Type mismatches", "Mismatch between target and source value types."));
					return {};
				}

				return TOptional<const void*>(PointerWrapper(PropertyContext.PropertyPtr, PropertyContext.ValuePtr.GetValue()));
			}
		}

		return {};
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
	virtual ~FAruPredicate_SetBoolValue() override {};
	virtual void Execute(FProperty* InProperty, void* InValue) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Float Value")
struct FAruPredicate_SetFloatValue : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	float NewValue = 0.f;
public:
	virtual ~FAruPredicate_SetFloatValue() override {};
	virtual void Execute(FProperty* InProperty, void* InValue) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Integer Value")
struct FAruPredicate_SetIntegerValue : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	int64 NewValue = 0;
public:
	virtual ~FAruPredicate_SetIntegerValue() override {};
	virtual void Execute(FProperty* InProperty, void* InValue) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Name Value")
struct FAruPredicate_SetNameValue : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	FName NewValue = FName{};
public:
	virtual ~FAruPredicate_SetNameValue() override {};
	virtual void Execute(FProperty* InProperty, void* InValue) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Struct Value")
struct FAruPredicate_SetStructValue : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	FInstancedStruct NewValue;
public:
	virtual ~FAruPredicate_SetStructValue() override {};
	virtual void Execute(FProperty* InProperty, void* InValue) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Object Value")
struct FAruPredicate_SetObjectValue : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	TObjectPtr<UObject> NewValue = nullptr;
public:
	virtual ~FAruPredicate_SetObjectValue() override {};
	virtual void Execute(FProperty* InProperty, void* InValue) const override;
};

USTRUCT(BlueprintType, DisplayName="Set Instanced Struct Value")
struct FAruPredicate_SetInstancedStructValue : public FAruPredicate_PropertySetter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ValueSource==EAruValueSource::Value", EditConditionHides))
	FInstancedStruct NewValue;
public:
	virtual ~FAruPredicate_SetInstancedStructValue() override {};
	virtual void Execute(FProperty* InProperty, void* InValue) const override;
};

#undef LOCTEXT_NAMESPACE