// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ModelMesh.h"
#include "RuntimeMeshActor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ModelMaterial.h"
#include "Providers/RuntimeMeshProviderStatic.h"
#include "RuntimeActor.generated.h"

UCLASS()
class RUNTIMEMODELIMPORT_API ARuntimeActor : public ARuntimeMeshActor
{
	GENERATED_BODY()
public:
	ARuntimeActor(const FObjectInitializer& Init);

	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

	virtual void Tick(float DeltaTime) override;

	//给Actor设置渲染的Mesh数据
	void SetModelMesh(TSharedPtr<FModelMesh> mesh);
	TSharedPtr<FModelMesh> GetModelMesh() { return ModelMesh; };

	void SetMaterialSlot(int32 MaterialSlot, FName SlotName, UMaterialInterface* InMaterial);

	void SetActorName(FString& strName);
	FString& GetActorName() { return ActorName; }
	void SetIsRoot(bool bRoot) { IsRoot = bRoot; }
	bool GetIsRoot() { return IsRoot; }

	UFUNCTION(BlueprintCallable)
	ARuntimeActor* GetRootActor();

	void Init(TSharedPtr<FModelMesh> mesh);

	//异步的
	void AsyncInit(TSharedPtr<FModelMesh> mesh);

	FBoxSphereBounds GetBoundingSphere() { return BoundingSphere; };

	UPROPERTY()
	FString GUID;

private:
	void traverseMeshTree(TSharedPtr<FModelMesh> mesh, AActor* pParent);

	void traverseActor(ARuntimeActor* rootActor);

	UMaterialInstanceDynamic* CreateMaterial(TSharedPtr<FModelMaterial> mat);

	TSharedPtr<FModelMaterial> ConvertMaterial(UMaterialInstanceDynamic*);

private:
	int meshCount = 0;
	int readIndex = 0;
	FBoxSphereBounds BoundingSphere;
	FString ActorName;
	bool IsRoot;
	TSharedPtr<FModelMesh> ModelMesh;
	FActorSpawnParameters SpawnInfo;
	FCriticalSection Mutex;
	UPROPERTY()
	URuntimeMeshProviderStatic* Provider;
};
