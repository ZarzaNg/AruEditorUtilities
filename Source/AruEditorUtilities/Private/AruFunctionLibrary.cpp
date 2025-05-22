#include "AruFunctionLibrary.h"

#include "AruTypes.h"
#include "EditorUtilityLibrary.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"

#define LOCTEXT_NAMESPACE "AruEditorUtilities"

void UAruFunctionLibrary::ProcessSelectedAssets(const TArray<FAruActionDefinition>& Actions, const FAruProcessConfig& Configs)
{
	const TArray<UObject*>&& SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	ProcessAssets(SelectedObjects, Actions, Configs);
}

void UAruFunctionLibrary::ProcessAssets(const TArray<UObject*>& Objects, const TArray<FAruActionDefinition>& Actions, const FAruProcessConfig& Configs)
{
	FScopedSlowTask Progress(Objects.Num(), LOCTEXT("Processing...", "Processing..."));
	Progress.MakeDialog();
	for (auto& Object : Objects)
	{
		Progress.EnterProgressFrame(1.f);

		UObject* ObjectToProcess = Object;
		UClass* ClassToProcess = Object->GetClass();
		if (UBlueprint* BlueprintAsset = Cast<UBlueprint>(Object))
		{
			ClassToProcess = BlueprintAsset->GeneratedClass;
			ObjectToProcess = ClassToProcess->GetDefaultObject();
		}

		bool bExecutedSuccessfully = false;
		for (TFieldIterator<FProperty> It{ClassToProcess}; It; ++It)
		{
			FProperty* Property = *It;
			if (Property == nullptr)
			{
				continue;
			}
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(ObjectToProcess);
			if (ValuePtr == nullptr)
			{
				continue;
			}

			bExecutedSuccessfully |= ProcessContainerValues(Property, ValuePtr, {Actions, Configs.Parameters, Configs.MaxSearchDepth});
		}

		if (bExecutedSuccessfully)
		{
			Object->Modify();
		}
	}
}

bool UAruFunctionLibrary::ProcessContainerValues(
	FProperty* PropertyPtr,
	void* ValuePtr,
	const FAruProcessingParameters& InParameters)
{
	if (InParameters.RemainTime <= 0 || PropertyPtr == nullptr || ValuePtr == nullptr)
	{
		return false;
	}

	bool bExecutedSuccessfully = false;
	if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(PropertyPtr))
	{
		[&]
		{
			UObject* NativeObject = ObjectProperty->GetObjectPropertyValue(ValuePtr);
			if (NativeObject == nullptr)
			{
				return;
			}

			UClass* NativeClass = NativeObject->GetClass();
			if (NativeClass == nullptr)
			{
				return;
			}

			if (UBlueprint* BlueprintAsset = Cast<UBlueprint>(NativeObject))
			{
				NativeClass = BlueprintAsset->GeneratedClass;
				NativeObject = NativeClass->GetDefaultObject();
			}

			for (TFieldIterator<FProperty> It{NativeClass}; It; ++It)
			{
				FProperty* Property = *It;
				if (Property == nullptr)
				{
					continue;
				}
				void* ObjectValuePtr = Property->ContainerPtrToValuePtr<void>(NativeObject);
				if (ValuePtr == nullptr)
				{
					continue;
				}
				bExecutedSuccessfully |= ProcessContainerValues(Property, ObjectValuePtr, InParameters.GetSubsequentParameters());
			}
		}();
	}
	else if (FStructProperty* StructProperty = CastField<FStructProperty>(PropertyPtr))
	{
		[&]
		{
			const UScriptStruct* StructType = StructProperty->Struct;
			if (StructType == nullptr)
			{
				return;
			}

			if (StructType == FGameplayTag::StaticStruct()
				|| StructType == FGameplayTagQuery::StaticStruct()
				|| StructType == FGameplayTagContainer::StaticStruct())
			{
				return;
			}

			if (StructType == FInstancedStruct::StaticStruct())
			{
				FInstancedStruct* InstancedStructPtr = static_cast<FInstancedStruct*>(ValuePtr);
				if (InstancedStructPtr == nullptr || !InstancedStructPtr->IsValid())
				{
					return;
				}
				const UScriptStruct* InstancedStructType = InstancedStructPtr->GetScriptStruct();
				if (InstancedStructType == nullptr)
				{
					return;
				}
				void* InstancedStructContainer = InstancedStructPtr->GetMutableMemory();
				if (InstancedStructContainer == nullptr)
				{
					return;
				}

				for (TFieldIterator<FProperty> It{InstancedStructType}; It; ++It)
				{
					FProperty* Property = *It;
					if (Property == nullptr)
					{
						continue;
					}
					void* StructValuePtr = Property->ContainerPtrToValuePtr<void>(InstancedStructContainer);
					if (ValuePtr == nullptr)
					{
						continue;
					}
					bExecutedSuccessfully |= ProcessContainerValues(Property, StructValuePtr, InParameters.GetSubsequentParameters());
				}
			}
			else
			{
				for (TFieldIterator<FProperty> It{StructType}; It; ++It)
				{
					FProperty* Property = *It;
					if (Property == nullptr)
					{
						continue;
					}
					void* StructValuePtr = Property->ContainerPtrToValuePtr<void>(ValuePtr);
					if (ValuePtr == nullptr)
					{
						continue;
					}
					bExecutedSuccessfully |= ProcessContainerValues(Property, StructValuePtr, InParameters.GetSubsequentParameters());
				}
			}
		}();
	}
	else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(PropertyPtr))
	{
		FScriptArrayHelper ArrayHelper{ArrayProperty, ValuePtr};
		for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
		{
			void* ItemPtr = ArrayHelper.GetRawPtr(Index);
			bExecutedSuccessfully |= ProcessContainerValues(ArrayProperty->Inner, ItemPtr, InParameters.GetSubsequentParameters());
		}
	}
	else if (FMapProperty* MapProperty = CastField<FMapProperty>(PropertyPtr))
	{
		FScriptMapHelper MapHelper{MapProperty, ValuePtr};
		for (int32 Index = 0; Index < MapHelper.Num(); ++Index)
		{
			void* MapKeyPtr = MapHelper.GetKeyPtr(Index);
			void* MapValuePtr = MapHelper.GetValuePtr(Index);
			bExecutedSuccessfully |= ProcessContainerValues(MapProperty->KeyProp, MapKeyPtr, InParameters.GetSubsequentParameters());
			bExecutedSuccessfully |= ProcessContainerValues(MapProperty->ValueProp, MapValuePtr, InParameters.GetSubsequentParameters());
		}
	}
	else if (FSetProperty* SetProperty = CastField<FSetProperty>(PropertyPtr))
	{
		FScriptSetHelper SetHelper{SetProperty, ValuePtr};
		for (int32 Index = 0; Index < SetHelper.Num(); ++Index)
		{
			void* ItemPtr = SetHelper.GetElementPtr(Index);
			bExecutedSuccessfully |= ProcessContainerValues(SetProperty->ElementProp, ItemPtr, InParameters.GetSubsequentParameters());
		}
	}

	for (const auto& Action : InParameters.Actions)
	{
		bExecutedSuccessfully |= Action.Invoke(PropertyPtr, ValuePtr, InParameters.Parameters);
	}

	return bExecutedSuccessfully;
}

FAruPropertyContext UAruFunctionLibrary::FindPropertyByPath(
	const FProperty* InProperty,
	const void* InPropertyValue,
	const FString& Path)
{
	if (InProperty == nullptr || InPropertyValue == nullptr || Path.IsEmpty())
	{
		return {};
	}

	TArray<FString> PropertyChain;
	Path.ParseIntoArray(PropertyChain, TEXT("."), true);

	return FindPropertyByChain(InProperty, InPropertyValue, PropertyChain);
}

FAruPropertyContext UAruFunctionLibrary::FindPropertyByPath(
	const UStruct* InStructType,
	const void* InStructValue,
	const FString& Path)
{
	if (InStructType == nullptr || InStructValue == nullptr || Path.IsEmpty())
	{
		return {};
	}

	TArray<FString> PropertyChain;
	Path.ParseIntoArray(PropertyChain, TEXT("."), true);
	if (PropertyChain.Num() == 0)
	{
		return {};
	}

	const FProperty* FirstProperty = InStructType->FindPropertyByName(*PropertyChain[0]);
	if (FirstProperty == nullptr)
	{
		return {};
	}

	const void* FirstPropertyValue = FirstProperty->ContainerPtrToValuePtr<void>(InStructValue);
	if (FirstPropertyValue == nullptr)
	{
		return {};
	}

	TArrayView<FString> RemainPropertyChain = TArrayView<FString>{PropertyChain}.RightChop(1);
	return FindPropertyByChain(FirstProperty, FirstPropertyValue, RemainPropertyChain);
}

FAruPropertyContext UAruFunctionLibrary::FindPropertyByChain(
	const FProperty* InProperty,
	const void* InPropertyValue,
	const TArrayView<FString> PropertyChain)
{
	const FProperty* CurrentProperty = InProperty;
	const void* CurrentPropertyValue = InPropertyValue;

	for (const FString& Element : PropertyChain)
	{
		// Make sure we have proper setup to go through the process.
		if (CurrentProperty == nullptr || CurrentPropertyValue == nullptr)
		{
			return {};
		}

		if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(CurrentProperty))
		{
			UObject* ObjectPtr = ObjectProperty->GetObjectPropertyValue(CurrentPropertyValue);
			if (ObjectPtr == nullptr)
			{
				return {};
			}

			const UClass* ClassType = ObjectPtr->GetClass();
			if (ClassType == nullptr)
			{
				return {};
			}

			CurrentProperty = ClassType->FindPropertyByName(*Element);
			if (CurrentProperty == nullptr)
			{
				// TODO: Add Log.
				return {};
			}

			// Ignore nullptr check. If the value is nullptr, we will return nullptr.
			CurrentPropertyValue = CurrentProperty->ContainerPtrToValuePtr<void>(ObjectPtr);
		}
		else if (const FStructProperty* StructProperty = CastField<FStructProperty>(CurrentProperty))
		{
			const UScriptStruct* StructType = StructProperty->Struct;
			if (StructType == nullptr)
			{
				return {};
			}

			if (StructType == FInstancedStruct::StaticStruct())
			{
				const FInstancedStruct* InstancedStructPtr = static_cast<const FInstancedStruct*>(CurrentPropertyValue);
				if (InstancedStructPtr == nullptr || !InstancedStructPtr->IsValid())
				{
					return {};
				}

				const UScriptStruct* InstancedStructType = InstancedStructPtr->GetScriptStruct();
				if (InstancedStructType == nullptr)
				{
					return {};
				}

				CurrentProperty = InstancedStructType->FindPropertyByName(*Element);
				if (CurrentProperty == nullptr)
				{
					// TODO: Add Log.
					return {};
				}

				const void* InstancedStructContainer = InstancedStructPtr->GetMemory();
				if (InstancedStructContainer == nullptr)
				{
					return {};
				}

				CurrentPropertyValue = CurrentProperty->ContainerPtrToValuePtr<void>(InstancedStructContainer);
				if (CurrentPropertyValue == nullptr)
				{
					return {};
				}
			}
			else
			{
				CurrentProperty = StructType->FindPropertyByName(*Element);
				if (CurrentProperty == nullptr)
				{
					// TODO: Add Log.
					return {};
				}

				CurrentPropertyValue = CurrentProperty->ContainerPtrToValuePtr<void>(CurrentPropertyValue);
				if (CurrentPropertyValue == nullptr)
				{
					return {};
				}
			}
		}
	}

	return FAruPropertyContext{const_cast<FProperty*>(CurrentProperty), const_cast<void*>(CurrentPropertyValue)};
}

#undef LOCTEXT_NAMESPACE
