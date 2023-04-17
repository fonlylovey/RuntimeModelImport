/**
 * reader 各种模型的接口类
 */

#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ModelMesh.h"

struct RUNTIMEMODELIMPORT_API FImportOptions
{
	FImportOptions();

	//是否按照材质合并模型
	bool MergeByMaterial = true;

	//是否按照类型合并模型
	bool MergeByClass = false;

	//true:使用模型材质/false:使用材质库
	bool UserModelMaterial = true;

	bool ShowProgress = true;
};


class IReaderInterface
{
public:
	//异步的了, 此时返回FModelMesh*根节点是没有数据的, 通过监听FRMIDelegates::OnMeshTreeBuildFinishDelegate()拿到读取完后的根节点
	//考虑到后续会做其他格式的读取, 暂时先不将FModelMesh*返回值改为void.
	virtual inline FModelMesh* ReadFile(const FString& strPath) = 0;
	virtual FModelMesh* ReadFile(const FString& strPath, const FImportOptions& options) = 0;
};
