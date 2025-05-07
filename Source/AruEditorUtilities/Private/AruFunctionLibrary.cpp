#include "AruFunctionLibrary.h"

#include "AruTypes.h"
#include "EditorUtilityLibrary.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"

#define LOCTEXT_NAMESPACE "AruEditorUtilities"

void UAruFunctionLibrary::ProcessSelectedAssets(const TArray<FAruActionDefinition>& ActionDefinitions, int32 MaxDepth)
{
	const TArray<UObject*>&& SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	ProcessAssets(SelectedObjects, ActionDefinitions, MaxDepth);
}

void UAruFunctionLibrary::ProcessAssets(const TArray<UObject*>& Objects, const TArray<FAruActionDefinition>& ActionDefinitions, int32 MaxDepth)
{
	FScopedSlowTask Progress(Objects.Num(), LOCTEXT("Processing...", "Processing..."));
	Progress.MakeDialog();
	for(auto& Object : Objects)
	{
		Progress.EnterProgressFrame(1.f);
		
		for(TFieldIterator<FProperty> It{Object->GetClass()}; It; ++It)  
		{  
			FProperty* Property = *It;  
			if(Property == nullptr)  
			{       
				continue;
			}    
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Object);  
			if(ValuePtr == nullptr)  
			{       
				continue;
			}
			
			ProcessContainerValues(Property, Object, ValuePtr, ActionDefinitions, MaxDepth);  
		}

		Object->Modify();
	}
}

void UAruFunctionLibrary::ProcessContainerValues(
	FProperty* PropertyPtr,
	void* ContainerPtr,
	void* ValuePtr,
	const TArray<FAruActionDefinition>& Actions,
	const uint8 RemainTimes)
{
	if(RemainTimes <= 0)
	{
		return;
	}
	
	if(PropertyPtr == nullptr || ContainerPtr == nullptr || ValuePtr == nullptr)  
    {       
    	return;  
    }

	for(const auto& Action : Actions)
	{
		Action.Invoke(PropertyPtr, ContainerPtr, ValuePtr);
	}
    
    if(FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(PropertyPtr))  
    {       
		UObject* NativeObject = ObjectProperty->GetObjectPropertyValue(ValuePtr);  
		if(NativeObject == nullptr)  
		{          
			return;  
		}

    	UClass* NativeClass = NativeObject->GetClass();
    	if(NativeClass == nullptr)
    	{
    		return;
    	}

    	if(UBlueprint* BlueprintAsset = Cast<UBlueprint>(NativeObject))
    	{
    		NativeClass = BlueprintAsset->GeneratedClass;
    		NativeObject = NativeClass->GetDefaultObject();
    	}

		for(TFieldIterator<FProperty> It{NativeClass}; It; ++It)  
		{          
			FProperty* Property = *It;  
			if(Property == nullptr)  
			{             
				continue;  
			}          
			void* ObjectValuePtr = Property->ContainerPtrToValuePtr<void>(NativeObject);  
			if(ValuePtr == nullptr)  
			{             
				continue;  
			}          
			ProcessContainerValues(Property, NativeObject, ObjectValuePtr, Actions, RemainTimes - 1);  
		}    
	}
	else if(FStructProperty* StructProperty = CastField<FStructProperty>(PropertyPtr))  
	{       
		const UScriptStruct* StructType = StructProperty->Struct;  
		if(StructType == nullptr)  
		{          
			return;  
		}
		
		if(StructType == FGameplayTag::StaticStruct() 
			|| StructType == FGameplayTagQuery::StaticStruct()
			|| StructType == FGameplayTagContainer::StaticStruct())
		{          
			return;  
		}
		
		if(StructType == FInstancedStruct::StaticStruct())  
		{          
			FInstancedStruct* InstancedStructPtr = static_cast<FInstancedStruct*>(ValuePtr);  
			if(InstancedStructPtr == nullptr || !InstancedStructPtr->IsValid())  
			{             
				return;  
			}  
			const UScriptStruct* InstancedStructType = InstancedStructPtr->GetScriptStruct();  
			if(InstancedStructType == nullptr)  
			{             
				return;  
			}  
			void* InstancedStructContainer = InstancedStructPtr->GetMutableMemory();  
			if(InstancedStructContainer == nullptr)  
			{             
				return;  
			}
			
			for(TFieldIterator<FProperty> It{InstancedStructType}; It; ++It)  
			{             
				FProperty* Property = *It;  
				if(Property == nullptr)  
				{                
					continue;  
				}          
				void* StructValuePtr = Property->ContainerPtrToValuePtr<void>(InstancedStructContainer);  
				if(ValuePtr == nullptr)  
				{                
					continue;  
				}          
				ProcessContainerValues(Property, InstancedStructContainer, StructValuePtr,Actions, RemainTimes - 1);      
			}  
		}       
		else  
		{
			for(TFieldIterator<FProperty> It{StructType}; It; ++It)  
			{             
				FProperty* Property = *It;  
				if(Property == nullptr)  
				{                
					continue;  
				}          
				void* StructValuePtr = Property->ContainerPtrToValuePtr<void>(ValuePtr);  
				if(ValuePtr == nullptr)  
				{                
					continue;  
				}          
				ProcessContainerValues(Property, ValuePtr, StructValuePtr, Actions, RemainTimes - 1);  
			}       
		}    
	}    
	else if(FArrayProperty* ArrayProperty = CastField<FArrayProperty>(PropertyPtr))  
	{       
		FScriptArrayHelper ArrayHelper{ArrayProperty, ValuePtr};  
		for(int32 Index = 0; Index < ArrayHelper.Num(); ++Index)  
		{          
			void* ItemPtr = ArrayHelper.GetRawPtr(Index);  
			ProcessContainerValues(ArrayProperty->Inner, ContainerPtr, ItemPtr, Actions, RemainTimes - 1);  
		}    
	}    
	else if(FMapProperty* MapProperty = CastField<FMapProperty>(PropertyPtr))  
	{       
		FScriptMapHelper MapHelper{MapProperty, ValuePtr};  
		for(int32 Index = 0; Index < MapHelper.Num(); ++Index)  
		{         
			void* MapKeyPtr = MapHelper.GetKeyPtr(Index);  
			void* MapValuePtr = MapHelper.GetValuePtr(Index);  
			ProcessContainerValues(MapProperty->KeyProp, ContainerPtr, MapKeyPtr, Actions, RemainTimes - 1);  
			ProcessContainerValues(MapProperty->ValueProp, ContainerPtr, MapValuePtr, Actions, RemainTimes - 1);  
		}    
	}    
	else if(FSetProperty* SetProperty = CastField<FSetProperty>(PropertyPtr))  
	{       
		FScriptSetHelper SetHelper{SetProperty, ValuePtr};  
		for(int32 Index = 0; Index < SetHelper.Num(); ++Index)  
		{          
			void* ItemPtr = SetHelper.GetElementPtr(Index);  
			ProcessContainerValues(SetProperty->ElementProp, ContainerPtr, ItemPtr, Actions, RemainTimes - 1);  
		}    
	}
}

FAruPropertyContext UAruFunctionLibrary::FindPropertyByPath(
	const FProperty* InProperty,
	const void* InPropertyPtr,
	const FString& Path)
{
	if(InProperty == nullptr || InPropertyPtr == nullptr || Path.IsEmpty())
	{
		return FAruPropertyContext{};
	}

	TArray<FString> PropertyChain;
	Path.ParseIntoArray(PropertyChain, TEXT("."), true);

	const FProperty* CurrentProperty = InProperty;
	const void*	CurrentPropertyPtr = InPropertyPtr;
	for (const FString& Element : PropertyChain)
	{
		if(const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(CurrentProperty))
		{
			UObject* ObjectPtr = ObjectProperty->GetObjectPropertyValue(CurrentPropertyPtr);
			if(ObjectPtr == nullptr)
			{
				return FAruPropertyContext{};
			}

			const UClass* ClassType = ObjectPtr->GetClass();
			if(ClassType == nullptr)
			{
				return FAruPropertyContext{};
			}

			CurrentProperty = ClassType->FindPropertyByName(*Element);
			if(CurrentProperty == nullptr)
			{
				return FAruPropertyContext{};
			}

			CurrentPropertyPtr = CurrentProperty->ContainerPtrToValuePtr<void>(ObjectPtr);
			if(CurrentPropertyPtr == nullptr)
			{
				return FAruPropertyContext{};
			}
		}
		else if(const FStructProperty* StructProperty = CastField<FStructProperty>(CurrentProperty))
		{
			const UScriptStruct* StructType = StructProperty->Struct;  
			if(StructType == nullptr)  
			{          
				return FAruPropertyContext{};  
			}

			if(StructType == FInstancedStruct::StaticStruct())
			{
				const FInstancedStruct* InstancedStructPtr = static_cast<const FInstancedStruct*>(CurrentPropertyPtr);  
				if(InstancedStructPtr == nullptr || !InstancedStructPtr->IsValid())  
				{             
					return FAruPropertyContext{};  
				}
				
				const UScriptStruct* InstancedStructType = InstancedStructPtr->GetScriptStruct();  
				if(InstancedStructType == nullptr)  
				{             
					return FAruPropertyContext{};  
				}

				CurrentProperty = InstancedStructType->FindPropertyByName(*Element);
				if(CurrentProperty == nullptr)
				{
					return FAruPropertyContext{};
				}
				
				const void* InstancedStructContainer = InstancedStructPtr->GetMemory();  
				if(InstancedStructContainer == nullptr)  
				{             
					return FAruPropertyContext{};  
				}

				CurrentPropertyPtr = CurrentProperty->ContainerPtrToValuePtr<void>(InstancedStructContainer);
				if(CurrentPropertyPtr == nullptr)
				{
					return FAruPropertyContext{};
				}
			}
			else
			{
				CurrentProperty = StructType->FindPropertyByName(*Element);
				if(CurrentProperty == nullptr)
				{
					return FAruPropertyContext{};
				}

				CurrentPropertyPtr = CurrentProperty->ContainerPtrToValuePtr<void>(CurrentPropertyPtr);
				if(CurrentPropertyPtr == nullptr)
				{
					return FAruPropertyContext{};
				}
			}
		}
		
		if(&Element == &PropertyChain.Last())
		{
			return FAruPropertyContext{const_cast<void*>(CurrentPropertyPtr), const_cast<FProperty*>(CurrentProperty)};
		}
	}

	return FAruPropertyContext{};
}

FAruPropertyContext UAruFunctionLibrary::FindPropertyByPath(
	const UStruct* InStructType,
	const void* InStructPtr,
	const FString& Path)
{
	if(InStructType == nullptr || InStructPtr == nullptr || Path.IsEmpty())
	{
		return FAruPropertyContext{};;
	}

	TArray<FString> PropertyChain;
	Path.ParseIntoArray(PropertyChain, TEXT("."), true);

	const FProperty* CurrentProperty = nullptr;
	const void* CurrentStructPtr = InStructPtr;
	const UStruct* CurrentStructType = InStructType;
	for (const FString& Element : PropertyChain)
	{
		CurrentProperty = CurrentStructType->FindPropertyByName(*Element);
		if(CurrentProperty == nullptr)
		{
			return FAruPropertyContext{};;
		}
		CurrentStructPtr = CurrentProperty->ContainerPtrToValuePtr<void>(CurrentStructPtr);
		if(CurrentStructPtr == nullptr)
		{
			return FAruPropertyContext{};;
		}

		if (&Element == &PropertyChain.Last())
		{
			break;
		}

		if(const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(CurrentProperty))
		{
			UObject* ObjectPtr = ObjectProperty->GetObjectPropertyValue(CurrentStructPtr);
			if(ObjectPtr == nullptr)
			{
				return FAruPropertyContext{};;
			}

			CurrentStructType = ObjectPtr->GetClass();
			if(CurrentStructType == nullptr)
			{
				return FAruPropertyContext{};;
			}
		}
		else if(const FStructProperty* StructProperty = CastField<FStructProperty>(CurrentProperty))
		{
			CurrentStructType = StructProperty->Struct;  
			if(CurrentStructType == nullptr)  
			{          
				return FAruPropertyContext{};  
			}

			if(CurrentStructType == FInstancedStruct::StaticStruct())
			{
				const FInstancedStruct* InstancedStructPtr = static_cast<const FInstancedStruct*>(CurrentStructPtr);  
				if(InstancedStructPtr == nullptr || !InstancedStructPtr->IsValid())  
				{             
					return FAruPropertyContext{};  
				}
				
				CurrentStructType = InstancedStructPtr->GetScriptStruct();
				if(CurrentStructType == nullptr)
				{
					return FAruPropertyContext{};
				}

				CurrentStructPtr = InstancedStructPtr->GetMemory();  
				if(CurrentStructPtr == nullptr)  
				{             
					return FAruPropertyContext{};  
				}
			}
		}
	}
	
	return FAruPropertyContext{const_cast<void*>(CurrentStructPtr), const_cast<FProperty*>(CurrentProperty)};
}

#undef LOCTEXT_NAMESPACE 
