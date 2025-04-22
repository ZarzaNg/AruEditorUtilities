#pragma once

#include "CoreMinimal.h"
#include "AruTypes.h"
#include "AruFunctionLibrary.generated.h"

struct FAruActionDefinition;

struct FAruPropertyContext
{
	void* ValuePtr			= nullptr;
	FProperty* PropertyPtr	= nullptr;
};

UCLASS()
class ARUEDITORUTILITIES_API UAruFunctionLibrary : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, CallInEditor)
	static void ProcessSelectedAssets(const FAruActionDefinition& ActionDefinition);
	
	static FAruPropertyContext FindPropertyByPath(
		const UStruct* Type,
		void* ContainerPtr,
		const FString& Path);
	
	static void ProcessContainerValues(
		FProperty* PropertyPtr,
		void* ContainerPtr,
		void* ValuePtr,
		FAruActionDefinition& Action,
		const uint8 RemainTimes);
};
