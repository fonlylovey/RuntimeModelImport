/**
 * reader 接口类
 */

#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ModelMesh.h"

namespace RuntimeDB
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadStartDelegate, const FString&);
	DECLARE_MULTICAST_DELEGATE_FourParams(FOnLoadingProgressDelegate, int, int32, int32, const FString&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadedCompleteDelegate, AActor*);
}

struct RUNTIMEMODELIMPORT_API FImportOptions
{
	FImportOptions();

	//按材质合并Section
	bool MergeByMaterial = true;

	//按类型合并材质
	bool MergeByClass = false;

	//true:使用模型材质，false：使用材质库材质
	bool UserModelMaterial = true;

	bool ShowProgress = true;
};


class FReaderBase
{
public:
	FReaderBase(const FString& FilePath);
	FReaderBase(const FString& FilePath, const FImportOptions& Options);
	virtual ~FReaderBase();
	//TSharedPtr<FModelMesh>, FRMIDelegates::OnMeshTreeBuildFinishDelegate()
	//, TSharedPtr<FModelMesh> void.
	virtual inline TSharedPtr<FModelMesh> ReadFile();
	virtual inline TSharedPtr<FModelMesh> ReadFile(const FString& FilePath);
	virtual TSharedPtr<FModelMesh> ReadFile(const FString& FilePath, const FImportOptions& Options) = 0;

	
	RuntimeDB::FOnLoadStartDelegate::FDelegate OnStartDelegate;
	RuntimeDB::FOnLoadStartDelegate::FDelegate OnLoadingProgressDelegate;
	RuntimeDB::FOnLoadStartDelegate::FDelegate OnFinishDelegate;

protected:
	FString ModelFilePath;
	FImportOptions ImportOptions;
};
