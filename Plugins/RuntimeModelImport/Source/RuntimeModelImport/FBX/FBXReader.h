// Fill out your copyright notice in the Description page of Project Settings.
/*
*	FBX 读取
*	读取FBX文本返回FModeMesh
*/
#pragma once
#include "fbxsdk.h"
#include "ModelMesh.h"
#include "FBXMeshImport.h"
#include "FBXMaterialImport.h"
#include "ReaderBase.h"

class FFBXReader : public FReaderBase
{
public:
	FFBXReader(const FString& FilePath);
	FFBXReader(const FString& FilePath, const FImportOptions& Options);
	
	virtual ~FFBXReader();

	//实现接口的函数, 此时是异步的, 返回值不可靠. 通过监听FRMIDelegates::OnMeshTreeBuildFinishDelegate()拿到读取完后的根节点
	//返回值占时没有意义，因为异步，会将结果广播出去
	inline  FModelMesh* ReadFile(const FString& strPath);
	FModelMesh* ReadFile(const FString& strPath, const FImportOptions& options);

private:
	void initFBXSDK();

	//坐标轴转换(未写实现代码)+模型单位化成虚幻的厘米
	void ConvertScene(FbxScene* pScene);

	//给Mesh关联材质
	//void ConncentMaterial(FModelMesh* pMesh, );

	FString GetFileName();


	//处理材质, 合并相关代码在这
	void PairMaterial();

	//关联材质不做任何处理
	void LinkMaterial(TSharedPtr <FModelMesh> pMesh);

	//关联材质，并且按材质合并Mesh
	void LinkAndMergeByMaterial();

	//fbximport 的回调函数
	static bool FbxImportCallback(void* pArgs, float pPercentage, const char* pStatus);

private:
	FString m_strModelPath;
	FbxManager* SdkManager;
	FbxScene* m_pFbxScene;
	FImportOptions m_ImportOption;

	int flag = 0;
	TSharedPtr <FModelMesh> m_pModelMesh;
	TSharedPtr<FBXMeshImport> m_pMeshImport;
	TSharedPtr<FBXMaterialImport> m_pMaterialImport;
	TFuture<void> m_initTask;
	TFuture<void> m_readMatTask;
	TFuture<void> m_readMeshTask;
	TFuture<void> m_mergeMatTask;
};
