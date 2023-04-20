/**
 * 
 */
#pragma once
#include "CoreMinimal.h"
#include "RuntimeActor.h"
#include "ReaderInterface.h"
#include "../SaveGame/ModelSaveSystem.h"

//��Ҫ�ĳ��̰߳�ȫ����
class FModelOperator
{
public:
	static FModelOperator* Instance();

	ARuntimeActor* ReadModelFile(const FString& strPath, const FImportOptions& options = FImportOptions());

	//����Actor������
	void SaveModel(const FString& saveDir, ARuntimeActor* actor);

	//load Ŀ¼�µ�����ģ��
	void LoadModel(const FString& fileDir);

	//����actor spawn���
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnSpawnCompleteDelegate, TArray<ARuntimeActor*>);
	static FOnSpawnCompleteDelegate OnSpawnComplete;

private:
	FModelOperator();
	virtual ~FModelOperator();

	//�����ļ���ʽ����ʵ�ʵ�reader
	TSharedPtr<IReaderInterface> CreateReader(const FString& fileSuffix);

	//��ȡ�ļ���׺������������ ��xxxx.png����png��
	FString GetSuffix(const FString& strPath);

	//����Mesh��״�ṹ���, Ŀ�����õ����ڵ��FModelMesh*����(�Ѿ�Build�õ�), ����ȥSpawnActor��
	void OnMeshTreeBuildFinishDelegateListen(FModelMesh* pRoot);

private:
	static FModelOperator* s_pSelf;

	TSharedPtr<IReaderInterface> m_pReader;

	FImportOptions m_option;

	ARuntimeActor* m_pModelActor;

	TSharedPtr<FModelSaveSystem> m_pSaveModelPtr;

	TArray<ARuntimeActor*> ActorList;
};

