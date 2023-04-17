// Fill out your copyright notice in the Description page of Project Settings.
/*
*	������
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
	*	��һ��
	*
	*	�ⲿ����Ҳ���Լ����ûص�, ���������֮�󴴽�һ���Լ��Ľ�����UI.
	*	PS:��������ò����UI, Ҫ��Importer::LoadModel(const FString& strPath, bool bShowBuiltInProgress)�����н�bShowBuiltInProgress��ֵΪfalse.
	*	
	*	��ʼ��ȡ
	*	����1: �ļ���
	*/
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnImportStartDelegate, const FString&);
	static FOnImportStartDelegate OnImportStartDelegate;

	/*
	*	�ڶ���
	*
	*	����ڲ��õ�, �ⲿ��Ҫ����
	*
	*	Mesh��״�ṹ���Ǵ�������, ����ȥSpawn��.
	*	����1: ���ڵ�
	*/
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnMeshTreeBuildFinishDelegate, FModelMesh*);
	static FOnMeshTreeBuildFinishDelegate OnMeshTreeBuildFinishDelegate;

	/*
	*	������
	*
	*	����ڲ��õ�, �ⲿ��Ҫ����
	*
	*	SpawnActor����
	*	����1: ���ڵ�
	*/
	DECLARE_MULTICAST_DELEGATE(FOnSpawnActorFinishDelegate);
	static FOnSpawnActorFinishDelegate OnSpawnActorFinishDelegate;

	/*
	*	��������н��ȵĸ���,������,��һ����ȡMesh, �ڶ���CreateSection.
	*
	*	�ⲿ����Ҳ���Լ����ûص������Լ��Ľ���UI, �ɲο�FRMIProgress::UpdateProgressUI���߼�. ERMIImportProgressStage����ö�ٷֱ�ռ��50%�Ľ���
	*	
	*	����1 ERMIImportProgressStage:��ǰ��ʲô�׶�
	*	����2 int32:��ǰ�±�
	*	����3 int32:����
	*	����4 const FString&:��ǰ�ڵ�Mesh������ 
	*/
	DECLARE_MULTICAST_DELEGATE_FourParams(FOnImportProgressDelegate, int, int32, int32, const FString&);
	static FOnImportProgressDelegate OnImportProgressDelegate;

	/*
	*	���һ��
	*
	*	�ⲿ����Ҳ���Լ����ûص�, ����ȡ��ɺ�����Ҫ��ʾ��Gameplay
	*	
	*	������ȡ, һ�������������(��+��ʾ)
	*	����1: �ļ���
	*/
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnImportCompleteDelegate, ARuntimeActor*);
	static FOnImportCompleteDelegate OnImportCompleteDelegate;
};
