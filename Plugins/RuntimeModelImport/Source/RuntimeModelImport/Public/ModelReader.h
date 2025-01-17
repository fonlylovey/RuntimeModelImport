﻿

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

	UFUNCTION(BlueprintCallable)
	void BeginLoad();

	UFUNCTION(BlueprintCallable)
	void AsyncBeginLoad();

	UFUNCTION(BlueprintCallable)
	void AsyncLoading();

	UFUNCTION(BlueprintCallable)
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
