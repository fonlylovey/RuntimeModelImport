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
	void SaveToFile(const FString& proDir, TSharedPtr<FModelMesh> mesh);

	void LoadByFile(const FString& proDir);

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadCompleteDelegate, const TArray<TSharedPtr<FModelMesh>>&);
	static FOnLoadCompleteDelegate OnLoadComplete;

	TArray<TSharedPtr<FModelMesh>> GetMeshList() { return MeshList; }
	//meshĿ¼�µ�����Mesh�ĸ��ڵ�, ��ʹ�õĵط�ȥ������飬��Ȼ���ü���û�����ӻᱻ����
	TArray<TSharedPtr<FModelMesh>> MeshList;

private:
	//��ModelMesh����ת�������ݣ����Ҽ����ڴ�����ݷֿ�
	void Traverse(TSharedPtr<FModelMesh> pMesh);

	UModelSaveGame* LoadMeshFile(const FString& filePath);

private:
	static UE::Tasks::FPipe ModelSaveAsyncPipe;

	FCriticalSection Mutex;

	//�ֿ�������
	TMap<int, TArray<FMeshData>> ChunkMap;
};
