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
	//�����ļ�������
	void SaveToFile(const FString& proDir, FModelMesh* mesh);

	void LoadByFile(const FString& proDir);

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadCompleteDelegate, TArray<TSharedPtr<FModelMesh>>);
	static FOnLoadCompleteDelegate OnLoadComplete;

private:
	//��ModelMesh����ת�������ݣ����Ҽ����ڴ�����ݷֿ�
	void Traverse(FModelMesh* pMesh, TMap<int, TArray<FMeshData>>& chunkMap);

	TFuture<UModelSaveGame*> loadMeshFIleAsync(const FString& filePath, TFunction<void()> CompletionCallback);

	UModelSaveGame* LoadMeshFile(const FString& filePath);

private:
	static UE::Tasks::FPipe ModelSaveAsyncPipe;

	//meshĿ¼�µ�����Mesh�ĸ��ڵ�
	TArray<TSharedPtr<FModelMesh>> MeshList;

	FCriticalSection Mutex;
};
