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
	else if(FWeakObjectProperty* WeakObjectProperty = CastField<FWeakObjectProperty>(PropertyPtr))
	{
		UObject* ObjectPtr = WeakObjectProperty->GetObjectPropertyValue(ValuePtr);
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
	else if(FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(PropertyPtr))
	{
		UObject* ObjectPtr = SoftObjectProperty->GetObjectPropertyValue(ValuePtr);
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
		
		if(&Element == &PropertyChain.Last())
		{
			return FAruPropertyContext{const_cast<void*>(CurrentPropertyPtr), const_cast<FProperty*>(CurrentProperty)};
		}
	}

	return FAruPropertyContext{};
}

#undef LOCTEXT_NAMESPACE 
