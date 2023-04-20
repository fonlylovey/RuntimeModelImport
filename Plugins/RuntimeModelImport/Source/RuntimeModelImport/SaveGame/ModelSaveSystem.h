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

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadCompleteDelegate, const TArray<TSharedPtr<FModelMesh>>&);
	static FOnLoadCompleteDelegate OnLoadComplete;

	TArray<TSharedPtr<FModelMesh>> GetMeshList() { return MeshList; }
	//mesh目录下的所有Mesh的根节点, 在使用的地方去清空数组，不然引用计数没有增加会被析构
	TArray<TSharedPtr<FModelMesh>> MeshList;

private:
	//将ModelMesh对象转换成数据，并且计算内存给数据分块
	void Traverse(FModelMesh* pMesh, TMap<int, TArray<FMeshData>>& chunkMap);

	UModelSaveGame* LoadMeshFile(const FString& filePath);

private:
	static UE::Tasks::FPipe ModelSaveAsyncPipe;

	FCriticalSection Mutex;
};
