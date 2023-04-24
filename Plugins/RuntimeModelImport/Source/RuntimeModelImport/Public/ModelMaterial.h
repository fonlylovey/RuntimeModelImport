// Fill out your copyright notice in the Description page of Project Settings.
//��ȡ�ĵ���FBXMeshת���������Լ��Ľṹ��
//��һ����״�Ľṹ
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

	//��͸���ȣ���͸�����෴������Դ�ڸ��ֽ�ģ���ߣ� Ϊ1ʱ ģ����ȫ��͸�� Ϊ0 ��ȫ͸��
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
