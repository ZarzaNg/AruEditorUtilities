#pragma once

#include "CoreMinimal.h"
#include "AruTypes.h"
#include "AruFunctionLibrary.generated.h"

struct FAruActionDefinition;

struct FAruPropertyContext
{
	FProperty* PropertyPtr		= nullptr;
	TOptional<void*> ValuePtr	= {};

	FAruPropertyContext() : PropertyPtr(nullptr), ValuePtr({}) {}
	FAruPropertyContext(FProperty* InPropertyPtr, void* InValuePtr) : PropertyPtr(InPropertyPtr), ValuePtr(TOptional<void*>{InValuePtr}) {} 

	FORCEINLINE bool IsValid() const { return PropertyPtr != nullptr && ValuePtr.IsSet(); }
};

UCLASS()
class ARUEDITORUTILITIES_API UAruFunctionLibrary : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, CallInEditor)
	static void ProcessSelectedAssets(const TArray<FAruActionDefinition>& ActionDefinitions, int32 MaxDepth);

	UFUNCTION(BlueprintCallable, CallInEditor)
	static void ProcessAssets(const TArray<UObject*>& Objects, const TArray<FAruActionDefinition>& ActionDefinitions, int32 MaxDepth = 5);

	static FAruPropertyContext FindPropertyByPath(
		const FProperty* InProperty,
		const void* InPropertyValue,
		const FString& Path);

	static FAruPropertyContext FindPropertyByPath(
		const UStruct* InStructType,
		const void* InStructValue,
		const FString& Path);

	static FAruPropertyContext FindPropertyByChain(
		const FProperty* InProperty,
		const void* InPropertyValue,
		const TArrayView<FString> PropertyChain);
	
	static void ProcessContainerValues(
		FProperty* PropertyPtr,
		void* ContainerPtr,
		void* ValuePtr,
		const TArray<FAruActionDefinition>& Actions,
		const uint8 RemainTimes);
};
