/*
 * 模型导入模块的入口类，所以导入模型功能都将从这个地方开始
 */
#pragma once
#include "CoreMinimal.h"
#include "RuntimeActor.h"
#include "ReaderInterface.h"


 //离开地图时候为true, 打开新的后为false
extern RUNTIMEMODELIMPORT_API bool GRMIStartToLevelMap;

namespace Importer
{
	// load fbx model
	extern RUNTIMEMODELIMPORT_API ARuntimeActor* LoadModel(const FString& strPath, const FImportOptions& options);
	
	//保存Actor到本地
	extern RUNTIMEMODELIMPORT_API void SaveModel(const FString& saveDir, ARuntimeActor* actor);

	//load 目录下的所有模型
	extern RUNTIMEMODELIMPORT_API TArray<ARuntimeActor*> LoadModel(const FString& saveDir);

}
