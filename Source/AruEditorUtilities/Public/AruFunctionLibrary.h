#pragma once

#include "CoreMinimal.h"
#include "AruTypes.h"
#include "AruFunctionLibrary.generated.h"

struct FAruActionDefinition;

struct FAruPropertyContext
{
	void* ValuePtr			= nullptr;
	FProperty* PropertyPtr	= nullptr;

	FORCEINLINE bool IsValid() const { return ValuePtr != nullptr && PropertyPtr != nullptr; }
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
		const void* InPropertyPtr,
		const FString& Path);

	static FAruPropertyContext FindPropertyByPath(
		const UStruct* InStructType,
		const void* InStructPtr,
		const FString& Path);
	
	static void ProcessContainerValues(
		FProperty* PropertyPtr,
		void* ContainerPtr,
		void* ValuePtr,
		const TArray<FAruActionDefinition>& Actions,
		const uint8 RemainTimes);
};
