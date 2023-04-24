// Fill out your copyright notice in the Description page of Project Settings.
//读取的单个FBXMesh转换到我们自己的结构上
//是一个树状的结构
#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshRenderable.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture.h"
#include "TextureResource.h"
#include "Engine/NetSerialization.h"
#include "ModelMaterial.generated.h"

USTRUCT(BlueprintType)
struct RUNTIMEMODELIMPORT_API FModelTexture
{
	GENERATED_USTRUCT_BODY();

	FModelTexture()
	{
		SizeX = 0;
		SizeY = 0;
	};

	~FModelTexture();


	UTexture2D* ToTexture(bool genMipmap = true);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SizeX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SizeY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<uint8> BulkData;
};


USTRUCT(BlueprintType)
struct RUNTIMEMODELIMPORT_API FModelMaterial
{
	GENERATED_USTRUCT_BODY();

	FModelMaterial();

	FModelMaterial(int32 id, FString name);
	~FModelMaterial();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsTransparent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DiffuseFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EmissiveFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Metallic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Roughness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Specular;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DepthOffset;

	//不透明度（和透明度相反，区别源于各种建模工具） 为1时 模型完全不透明 为0 完全透明
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Opacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DiffuseColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor EmissiveColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FModelTexture DiffuseMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FModelTexture NormalMap;
};
