#include "AruFunctionLibrary.h"

#include "AruTypes.h"
#include "EditorUtilityLibrary.h"
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
			
			ProcessContainerValues(Property, Object, ValuePtr, const_cast<TArray<FAruActionDefinition>&>(ActionDefinitions), MaxDepth);  
		}

		Object->Modify();
	}
}

void UAruFunctionLibrary::ProcessContainerValues(
	FProperty* PropertyPtr,
	void* ContainerPtr,
	void* ValuePtr,
	TArray<FAruActionDefinition>& Actions,
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

	for(auto& Action : Actions)
	{
		Action.Invoke(PropertyPtr, ContainerPtr, ValuePtr);
	}
    
    if(FObjectProperty* ObjectProperty = CastField<FObjectProperty>(PropertyPtr))  
    {       
		UObject* ObjectPtr = ObjectProperty->GetObjectPropertyValue(ValuePtr);  
		if(ObjectPtr == nullptr)  
		{          
			return;  
		}
    	
    	const UClass* ClassType = ObjectPtr->GetClass();
    	if(ClassType == nullptr)
    	{
    		return;
    	}

		for(TFieldIterator<FProperty> It{ObjectPtr->GetClass()}; It; ++It)  
		{          
			FProperty* Property = *It;  
			if(Property == nullptr)  
			{             
				continue;  
			}          
			void* ObjectValuePtr = Property->ContainerPtrToValuePtr<void>(ObjectPtr);  
			if(ValuePtr == nullptr)  
			{             
				continue;  
			}          
			ProcessContainerValues(Property, ObjectPtr, ObjectValuePtr, Actions, RemainTimes - 1);  
		}    
	}    
	else if(FStructProperty* StructProperty = CastField<FStructProperty>(PropertyPtr))  
	{       
		const UScriptStruct* StructType = StructProperty->Struct;  
		if(StructType == nullptr)  
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
	const UStruct* Type,
	void* ContainerPtr,
	const FString& Path)
{
	if(Type == nullptr || ContainerPtr == nullptr || Path.IsEmpty())
	{
		return FAruPropertyContext{};
	}

	TArray<FString> PropertyChain;
	Path.ParseIntoArray(PropertyChain, TEXT("."), true);

	const UStruct* CurrentStruct = Type;
	void* CurrentValue = ContainerPtr;
	FAruPropertyContext Context{};
	for (const FString& Element : PropertyChain)
	{
		FProperty* CurrentProperty = CurrentStruct->FindPropertyByName(*Element);
		if (CurrentProperty == nullptr)
		{
			break;
		}

		if (const FObjectProperty* ObjectProp = CastField<FObjectProperty>(CurrentProperty))
		{
			CurrentValue = ObjectProp->GetObjectPropertyValue_InContainer(CurrentValue);
			CurrentStruct = ObjectProp->PropertyClass;
		}
		else if (const FStructProperty* StructProp = CastField<FStructProperty>(CurrentProperty))
		{
			CurrentValue = StructProp->ContainerPtrToValuePtr<void>(CurrentValue);
			CurrentStruct = StructProp->Struct;
		}
		else
		{
			if (&Element != &PropertyChain.Last())
			{
				CurrentProperty = nullptr;
				break;
			}
			
			Context.PropertyPtr = CurrentProperty;
			Context.ValuePtr = CurrentProperty->ContainerPtrToValuePtr<void>(CurrentValue);
		}
	}

	return Context;
}

#undef LOCTEXT_NAMESPACE 