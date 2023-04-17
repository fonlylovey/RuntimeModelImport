// Fill out your copyright notice in the Description page of Project Settings.
//��ȡ�ĵ���FBXMeshת���������Լ��Ľṹ��
//��һ����״�Ľṹ
#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshRenderable.h"
#include "ModelMaterial.h"


struct RUNTIMEMODELIMPORT_API FModelMesh : public TSharedFromThis<FModelMesh, ESPMode::ThreadSafe>
{
	FModelMesh();
	FModelMesh(const FModelMesh&) = default;
	FModelMesh(int32 ID, FString strName);
	~FModelMesh();

	//���ݴ����FModelMesh�õ�Children������, bRecursion �Ƿ�ݹ�
	int32 GetChildrenNum(bool bRecursion = false);

	//��Mesh�ṹǶ��/�������ʺϲ���ʱ��Ҳ������ƽ�нṹ
	const TArray<TSharedPtr<FModelMesh, ESPMode::ThreadSafe>>& GetChildren() { return Children; }

	//ID
	int32 MeshID;

	//Name
	FString MeshName;

	FString MeshGUID;

	//��ID
	int32 ParentID;

	FModelMesh* Parent;

	//��ʶ�Ƿ���ڵ�
	bool IsRoot;

	//Mesh��������Ϣ
	FTransform MeshMatrix;
	
	//��Mesh�ṹǶ��/�������ʺϲ���ʱ��Ҳ������ƽ�нṹ
	TArray<TSharedPtr<FModelMesh, ESPMode::ThreadSafe>> Children;

	//ת����RuntimeMesh��Data
	TArray< TSharedPtr<FRuntimeMeshSectionData>> SectionList;

	//���ʲ�����, ͬ��Ҳ��Ҫ�ͷ�
	//TArray< TSharedPtr<FRuntimeMeshMaterialSlot>> MaterialSlotList;

	TArray<FModelMaterial> MaterialList;

};