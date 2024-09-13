

#pragma once

#include "CoreMinimal.h"
#include "ModelMesh.h"
#include "ReaderBase.h"
#include "UObject/Object.h"
#include "ModelReader.generated.h"

/**
 * 
 */

UCLASS(BlueprintType)
class RUNTIMEMODELIMPORT_API UModelReader : public UObject
{
	GENERATED_BODY()

public:
	UModelReader(const FObjectInitializer& ObjectInitializer);
	virtual ~UModelReader();

	void Init(const FString& FilePath);
	
	FModelMesh* ReadFile();

	UFUNCTION(BlueprintCallable)
	void LoadModel();
	
	void AsyncBeginLoad();

	void AsyncLoading();
	
	void AsyncFinishLoad();
	
private:
	void OnStartEvent( const FString& info);

public:
	RuntimeDB::FOnLoadStartDelegate OnStart;
	
	RuntimeDB::FOnLoadingProgressDelegate OnLoadingProgress;
	
	RuntimeDB::FOnLoadedCompleteDelegate OnFinish;
	
	
private:
	TSharedPtr<FReaderBase> ReaderPrivate;

	
};
