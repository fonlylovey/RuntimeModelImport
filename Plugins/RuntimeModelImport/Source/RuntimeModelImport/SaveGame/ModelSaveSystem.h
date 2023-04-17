#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "Tasks/Pipe.h"
#include "ModelSaveGame.h"
//#include "ModelSaveSystem.generated.h"

class FModelSaveSystem 
{
	//GENERATED_BODY()
public:
	//保存文件到本地
	void SaveToFile(const FString& proDir, FModelMesh* mesh);

	void LoadByFile(const FString& proDir);

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadCompleteDelegate, TArray<TSharedPtr<FModelMesh>>);
	static FOnLoadCompleteDelegate OnLoadComplete;

private:
	//将ModelMesh对象转换成数据，并且计算内存给数据分块
	void Traverse(FModelMesh* pMesh, TMap<int, TArray<FMeshData>>& chunkMap);

	TFuture<UModelSaveGame*> loadMeshFIleAsync(const FString& filePath, TFunction<void()> CompletionCallback);

	UModelSaveGame* LoadMeshFile(const FString& filePath);

private:
	static UE::Tasks::FPipe ModelSaveAsyncPipe;

	//mesh目录下的所有Mesh的根节点
	TArray<TSharedPtr<FModelMesh>> MeshList;

	FCriticalSection Mutex;
};
