// Fill out your copyright notice in the Description page of Project Settings.
/*
*	FBX ��ȡ
*	��ȡFBX�ı�����FModeMesh
*/
#pragma once
#include "fbxsdk.h"
#include "ModelMesh.h"
#include "FBXMeshImport.h"
#include "FBXMaterialImport.h"
#include "ReaderInterface.h"

class FBXReader : public IReaderInterface
{
public:
	FBXReader();
	virtual ~FBXReader();

	//ʵ�ֽӿڵĺ���, ��ʱ���첽��, ����ֵ���ɿ�. ͨ������FRMIDelegates::OnMeshTreeBuildFinishDelegate()�õ���ȡ���ĸ��ڵ�
	//����ֵռʱû�����壬��Ϊ�첽���Ὣ����㲥��ȥ
	inline  FModelMesh* ReadFile(const FString& strPath);
	FModelMesh* ReadFile(const FString& strPath, const FImportOptions& options);

private:
	void initFBXSDK();

	//������ת��(δдʵ�ִ���)+ģ�͵�λ������õ�����
	void ConvertScene(FbxScene* pScene);

	//��Mesh��������
	//void ConncentMaterial(FModelMesh* pMesh, );

	FString GetFileName();


	//�������, �ϲ���ش�������
	void PairMaterial();

	//�������ʲ����κδ���
	void LinkMaterial(FModelMesh* pMesh);

	//�������ʣ����Ұ����ʺϲ�Mesh
	void LinkAndMergeByMaterial();

	//fbximport �Ļص�����
	static bool FbxImportCallback(void* pArgs, float pPercentage, const char* pStatus);

private:
	FString m_strModelPath;
	FbxManager* SdkManager;
	FbxScene* m_pFbxScene;
	FImportOptions m_ImportOption;

	int flag = 0;
	FModelMesh* m_pModelMesh;
	TSharedPtr<FBXMeshImport> m_pMeshImport;
	TSharedPtr<FBXMaterialImport> m_pMaterialImport;
	TFuture<void> m_initTask;
	TFuture<void> m_readMatTask;
	TFuture<void> m_readMeshTask;
	TFuture<void> m_mergeMatTask;
};
