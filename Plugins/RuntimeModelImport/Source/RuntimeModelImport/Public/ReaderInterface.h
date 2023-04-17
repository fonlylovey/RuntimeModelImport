/**
 * reader ����ģ�͵Ľӿ���
 */

#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ModelMesh.h"

struct RUNTIMEMODELIMPORT_API FImportOptions
{
	FImportOptions();

	//�Ƿ��ղ��ʺϲ�ģ��
	bool MergeByMaterial = true;

	//�Ƿ������ͺϲ�ģ��
	bool MergeByClass = false;

	//true:ʹ��ģ�Ͳ���/false:ʹ�ò��ʿ�
	bool UserModelMaterial = true;

	bool ShowProgress = true;
};


class IReaderInterface
{
public:
	//�첽����, ��ʱ����FModelMesh*���ڵ���û�����ݵ�, ͨ������FRMIDelegates::OnMeshTreeBuildFinishDelegate()�õ���ȡ���ĸ��ڵ�
	//���ǵ���������������ʽ�Ķ�ȡ, ��ʱ�Ȳ���FModelMesh*����ֵ��Ϊvoid.
	virtual inline FModelMesh* ReadFile(const FString& strPath) = 0;
	virtual FModelMesh* ReadFile(const FString& strPath, const FImportOptions& options) = 0;
};
