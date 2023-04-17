// Fill out your copyright notice in the Description page of Project Settings.
/*
*	代理定义
*/
#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "RMITypes.h"
#include "RuntimeActor.h"

class ARuntimeActor;
struct FModelMesh;
class RUNTIMEMODELIMPORT_API FRMIDelegates
{
public:
	/*
	*	第一步
	*
	*	外部函数也可以监听该回调, 比如监听到之后创建一个自己的进度条UI.
	*	PS:如果不想用插件的UI, 要在Importer::LoadModel(const FString& strPath, bool bShowBuiltInProgress)方法中将bShowBuiltInProgress传值为false.
	*	
	*	开始读取
	*	参数1: 文件名
	*/
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnImportStartDelegate, const FString&);
	static FOnImportStartDelegate OnImportStartDelegate;

	/*
	*	第二步
	*
	*	插件内部用的, 外部不要调用
	*
	*	Mesh树状结构算是处理完了, 可以去Spawn了.
	*	参数1: 根节点
	*/
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnMeshTreeBuildFinishDelegate, FModelMesh*);
	static FOnMeshTreeBuildFinishDelegate OnMeshTreeBuildFinishDelegate;

	/*
	*	第三步
	*
	*	插件内部用的, 外部不要调用
	*
	*	SpawnActor结束
	*	参数1: 根节点
	*/
	DECLARE_MULTICAST_DELEGATE(FOnSpawnActorFinishDelegate);
	static FOnSpawnActorFinishDelegate OnSpawnActorFinishDelegate;

	/*
	*	导入过程中进度的更新,分两步,第一步读取Mesh, 第二步CreateSection.
	*
	*	外部函数也可以监听该回调来做自己的进度UI, 可参考FRMIProgress::UpdateProgressUI的逻辑. ERMIImportProgressStage两个枚举分别占用50%的进度
	*	
	*	参数1 ERMIImportProgressStage:当前是什么阶段
	*	参数2 int32:当前下标
	*	参数3 int32:总数
	*	参数4 const FString&:当前节点Mesh的名字 
	*/
	DECLARE_MULTICAST_DELEGATE_FourParams(FOnImportProgressDelegate, int, int32, int32, const FString&);
	static FOnImportProgressDelegate OnImportProgressDelegate;

	/*
	*	最后一步
	*
	*	外部函数也可以监听该回调, 做读取完成后你想要显示的Gameplay
	*	
	*	结束读取, 一个导入流程完毕(读+显示)
	*	参数1: 文件名
	*/
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnImportCompleteDelegate, ARuntimeActor*);
	static FOnImportCompleteDelegate OnImportCompleteDelegate;
};
