// Fill out your copyright notice in the Description page of Project Settings.
//读取的单个FBXMesh转换到我们自己的结构上
//是一个树状的结构
#pragma once

#include "CoreMinimal.h"
#include "ModelMaterial.h"
#include "ModelSection.h"
#include "ModelMesh.generated.h"

USTRUCT(BlueprintType)
struct RUNTIMEMODELIMPORT_API FModelMesh
{
	GENERATED_BODY()
public:
	FModelMesh();
	FModelMesh(const FModelMesh&) = default;
	FModelMesh(int32 ID, FString strName);
	~FModelMesh();

	//根据传入的FModelMesh拿到Children的数量, bRecursion 是否递归
	int32 GetChildrenNum(bool bRecursion = false);

	//子Mesh结构嵌套/当按材质合并的时候也可能是平行结构
	const TArray<TSharedPtr<FModelMesh, ESPMode::ThreadSafe>>& GetChildren() { return Children; }

	//ID
	int32 MeshID;

	//Name
	FString MeshName;

	FString MeshGUID;

	//父ID
	int32 ParentID;

	TSharedPtr<FModelMesh> Parent;

	TArray<TSharedPtr<FModelSection, ESPMode::ThreadSafe>> Sections;
	
	//标识是否根节点
	bool IsRoot;

	//Mesh的坐标信息
	FTransform MeshMatrix;
	
	//子Mesh结构嵌套/当按材质合并的时候也可能是平行结构
	TArray<TSharedPtr<FModelMesh, ESPMode::ThreadSafe>> Children;

	//材质数据
	TArray<TSharedPtr<FModelMaterial>> MaterialList;
};