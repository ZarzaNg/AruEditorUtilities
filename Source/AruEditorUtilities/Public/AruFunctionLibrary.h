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

struct FAruProcessingParameters
{
	const TArray<FAruActionDefinition>&		Actions;
	const FInstancedPropertyBag&			Parameters;
	const int32								RemainTime;

	FAruProcessingParameters() = delete;
	FAruProcessingParameters(
		const TArray<FAruActionDefinition>& InActions,
		const FInstancedPropertyBag&		InParameters,
		const int32							InRemainTime)
			: Actions(InActions), Parameters(InParameters), RemainTime(InRemainTime){};

	FAruProcessingParameters GetSubsequentParameters() const
	{
		return {Actions, Parameters, RemainTime-1};
	} 
};

UCLASS()
class ARUEDITORUTILITIES_API UAruFunctionLibrary : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, CallInEditor)
	static void ProcessSelectedAssets(const TArray<FAruActionDefinition>& Actions, const FAruProcessConfig& Configs);

	UFUNCTION(BlueprintCallable, CallInEditor)
	static void ProcessAssets(const TArray<UObject*>& Objects, const TArray<FAruActionDefinition>& Actions, const FAruProcessConfig& Configs);

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
	
	static bool ProcessContainerValues(
		FProperty* PropertyPtr,
		void* ValuePtr,
		const FAruProcessingParameters& InParameters);
};