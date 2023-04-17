/*
 * ģ�͵���ģ�������࣬���Ե���ģ�͹��ܶ���������ط���ʼ
 */
#pragma once
#include "CoreMinimal.h"
#include "RuntimeActor.h"
#include "ReaderInterface.h"


 //�뿪��ͼʱ��Ϊtrue, ���µĺ�Ϊfalse
extern RUNTIMEMODELIMPORT_API bool GRMIStartToLevelMap;

namespace Importer
{
	// load fbx model
	extern RUNTIMEMODELIMPORT_API ARuntimeActor* LoadModel(const FString& strPath, const FImportOptions& options);
	
	//����Actor������
	extern RUNTIMEMODELIMPORT_API void SaveModel(const FString& saveDir, ARuntimeActor* actor);

	//load Ŀ¼�µ�����ģ��
	extern RUNTIMEMODELIMPORT_API TArray<ARuntimeActor*> LoadModel(const FString& saveDir);

}
