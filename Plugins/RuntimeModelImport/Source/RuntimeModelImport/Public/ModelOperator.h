/**
 * 
 */
#pragma once
#include "CoreMinimal.h"
#include "RuntimeActor.h"
#include "ReaderInterface.h"
#include "../SaveGame/ModelSaveSystem.h"

//需要改成线程安全的类
class FModelOperator
{
public:
	static FModelOperator* Instance();

	ARuntimeActor* ReadModelFile(const FString& strPath, const FImportOptions& options = FImportOptions());

	//保存Actor到本地
	void SaveModel(const FString& saveDir, ARuntimeActor* actor);

	//load 目录下的所有模型
	void LoadModel(const FString& fileDir);

	//所有actor spawn完成
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnSpawnCompleteDelegate, TArray<ARuntimeActor*>);
	static FOnSpawnCompleteDelegate OnSpawnComplete;

private:
	FModelOperator();
	virtual ~FModelOperator();

	//根据文件格式创建实际的reader
	TSharedPtr<IReaderInterface> CreateReader(const FString& fileSuffix);

	//获取文件后缀名，不包含点 （xxxx.png返回png）
	FString GetSuffix(const FString& strPath);

	//监听Mesh树状结构完成, 目的是拿到根节点的FModelMesh*对象(已经Build好的), 可以去SpawnActor了
	void OnMeshTreeBuildFinishDelegateListen(FModelMesh* pRoot);

private:
	static FModelOperator* s_pSelf;

	TSharedPtr<IReaderInterface> m_pReader;

	FImportOptions m_option;

	ARuntimeActor* m_pModelActor;

	TSharedPtr<FModelSaveSystem> m_pSaveModelPtr;

	TArray<ARuntimeActor*> ActorList;
};

