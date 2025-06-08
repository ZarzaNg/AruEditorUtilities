#pragma once
#include "AruTypes.h"
#include "AruPredicate_LoadAssetByPath.generated.h"

USTRUCT(BlueprintType, DisplayName="Load Asset By Path")
struct FAruPredicate_LoadAssetByPath : public FAruPredicate
{
	GENERATED_BODY()

protected:
	/*
	 * The correct path configuration would be '/Game/Textures/MyTexture.MyTexture'.
	 * Alternatively, you can access the path to the resource by selecting 'Copy Object Path' in the editor.
	 */
	UPROPERTY(EditDefaultsOnly)
	FString PathToAsset{};

public:
	virtual ~FAruPredicate_LoadAssetByPath() override {};

	virtual bool Execute(const FProperty* InProperty, void* InValue, const FInstancedPropertyBag& InParameters) const override;
	
private:
	static FString GetCompactName() { return {"LoadAsset"}; }
};
