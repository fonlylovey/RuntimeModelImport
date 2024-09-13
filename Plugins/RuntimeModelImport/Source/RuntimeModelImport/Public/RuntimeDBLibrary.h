

#pragma once

#include "CoreMinimal.h"
#include "ModelReader.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RuntimeDBLibrary.generated.h"

/**
 * 
 */
UCLASS()
class RUNTIMEMODELIMPORT_API URuntimeDBLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "RuntimeModelImport")
	static UModelReader* MakeReader(const FString& FilePath);
};
