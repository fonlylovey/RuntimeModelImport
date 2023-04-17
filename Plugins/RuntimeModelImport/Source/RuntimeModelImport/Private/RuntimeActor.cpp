﻿#include "RuntimeActor.h"
#include "Modifiers/RuntimeMeshModifierNormals.h"
#include "Modifiers/RuntimeMeshModifierAdjacency.h"
#include <RMIDelegates.h>
#include "ModelOperator.h"
#include "Async/Async.h"
#include "Logging/LogMacros.h"

ARuntimeActor::ARuntimeActor(const FObjectInitializer& Init)
	: Super(Init)
	, IsRoot(false)
	, Provider(nullptr)
{
	ModelMesh = nullptr;
	Provider = NewObject<URuntimeMeshProviderStatic>();
	GetRuntimeMeshComponent()->Initialize(Provider);
	GetRuntimeMeshComponent()->Mobility = EComponentMobility::Movable;
	GetRuntimeMeshComponent()->SetRelativeTransform(FTransform());
	SpawnInfo.bNoFail;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}

void ARuntimeActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ARuntimeActor::SetModelMesh(FModelMesh* mesh)
{
	ModelMesh = mesh;
	ActorName = mesh->MeshName;
	IsRoot = mesh->IsRoot;
	
#if WITH_EDITOR
	SetActorLabel(ActorName);
#endif
	readIndex = 0;
	meshCount = mesh->GetChildrenNum();;

	SetActorTransform(ModelMesh->MeshMatrix);
	traverseMeshTree(mesh, this);
}

void ARuntimeActor::Init(FModelMesh* mesh)
{
	FString ueName = mesh->MeshName;
	GetRuntimeMeshComponent()->AppendName(ueName);
	if (Provider != nullptr)
	{
		FScopeLock ScopeLock(&Mutex);
		for (int i = 0; i < mesh->SectionList.Num(); i++)
		{
			TSharedPtr <FRuntimeMeshSectionData> section = mesh->SectionList[i];
			section->Properties.bCastsShadow = true;
			section->Properties.bIsVisible = true;
			mesh->MaterialList.Num() >= i ?
				section->Properties.MaterialSlot = i :
				section->Properties.MaterialSlot = 0;
			section->Properties.bIsMainPassRenderable = true;
			section->Properties.bWants32BitIndices = true;
			section->Properties.UpdateFrequency = ERuntimeMeshUpdateFrequency::Average;
			URuntimeMeshModifierNormals::CalculateNormalsTangents(section->MeshData);
			Provider->CreateSection(0, i, section->Properties, section->MeshData);

			BoundingSphere = Provider->GetBounds();
			ARuntimeActor* parent = Cast<ARuntimeActor>(GetOwner());
			if (parent != nullptr)
			{
				parent->BoundingSphere = parent->BoundingSphere + BoundingSphere;
			}
		}

		for (int i = 0; i < mesh->MaterialList.Num(); i++)
		{
			UMaterialInterface* mat = CreateMaterial(mesh->MaterialList[i]);
			if (mat)
				Provider->SetupMaterialSlot(i, FName(mesh->MaterialList[i].Name), mat);
		}

		//进度广播
		//Async(EAsyncExecution::TaskGraphMainThread, [&]()
		//	{
				++readIndex;
				FString strInfo = TEXT("生成Actor: " + mesh->MeshName);
				FRMIDelegates::OnImportProgressDelegate.Broadcast(1, readIndex, meshCount, strInfo);

				if (readIndex >= meshCount)
				{
					FRMIDelegates::OnSpawnActorFinishDelegate.Broadcast();
					FRMIDelegates::OnImportProgressDelegate.Broadcast(1, 1, 1, TEXT("导入完成"));
				}
				
		//	});
	}
}

void ARuntimeActor::AsyncInit(FModelMesh* mesh)
{
	//auto lambda = ;
	//Async(EAsyncExecution::ThreadPool, [&, mesh]() {
		Init(mesh);
	//	});
	//AsyncPool(*FQueuedThreadPool::Allocate(), lambda, nullptr, EQueuedWorkPriority::High);
	//GEKThread::GetPoolTask().CreateAsyncLambda(lambda);
}

void ARuntimeActor::traverseMeshTree(FModelMesh* mesh, AActor* pParent)
{
	if (!mesh->IsRoot)
	{
		UWorld* world = GetWorld();
		ARuntimeActor* fbxActor = world->SpawnActor<ARuntimeActor>(ARuntimeActor::StaticClass(),
			FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
		fbxActor->SetActorName(mesh->MeshName);
		fbxActor->SetIsRoot(mesh->IsRoot);
#if WITH_EDITOR
		fbxActor->SetActorLabel(mesh->MeshName);
#endif
		if (fbxActor == nullptr)
		{
			UE_LOG(LogTemp, Error, L"创建AFBXActor失败！");
			return;
		}

		fbxActor->Init(mesh);
		fbxActor->SetOwner(pParent);
		fbxActor->AttachToActor(pParent, FAttachmentTransformRules::KeepRelativeTransform);
		fbxActor->SetActorLocation(FVector::ZeroVector);
		pParent = fbxActor;
	}
	
	if (mesh->Children.Num() > 0)
	{
		for (auto pShareMesh : mesh->Children)
		{
			traverseMeshTree(pShareMesh.Get(), pParent);
		}
	}
}


void ARuntimeActor::traverseActor(ARuntimeActor* rootActor)
{
	if (IsRoot)
	{
		rootActor = this;
	}
	else
	{
		traverseActor(rootActor->GetOwner<ARuntimeActor>());
	}
}

UMaterialInterface* ARuntimeActor::CreateMaterial(FModelMaterial& mat)
{
	//此处有个析构的问题待解决, 先用这个方法检测一下是否崩溃
	FSoftObjectPath matPath;
	UMaterialInstanceDynamic* dynamicMaterial = nullptr;
	if (mat.Opacity == 1.0)
	{
		matPath = FSoftObjectPath(RMI_DEFAULTMAT);
	}
	else
	{
		matPath = FSoftObjectPath(RMI_TRANSPARENTMAT);
	}
	UObject* itemObj = matPath.ResolveObject();
	UMaterialInterface* baseMaterial = Cast<UMaterialInterface>(itemObj);
	dynamicMaterial = UMaterialInstanceDynamic::Create(baseMaterial, nullptr, FName(mat.Name));

	if (!dynamicMaterial)
		return nullptr;

	if (mat.DiffuseMap.BulkData.Num() > 0)
	{
		dynamicMaterial->SetTextureParameterValue(FName("DiffuseTexture"), mat.DiffuseMap.ToTexture());
	}
	else
	{
		dynamicMaterial->SetVectorParameterValue(FName("DiffuseColor"), mat.DiffuseColor);
	}

	if (mat.NormalMap.BulkData.Num() > 0)
	{
		dynamicMaterial->SetTextureParameterValue(FName("NormalTexture"), mat.NormalMap.ToTexture());
	}

	dynamicMaterial->SetVectorParameterValue(FName("EmissiveColor"), mat.EmissiveColor);
	dynamicMaterial->SetScalarParameterValue(FName("EmissiveFactor"), mat.EmissiveFactor);

	if(mat.Opacity == 1.0)
	{
		dynamicMaterial->SetScalarParameterValue(FName("DiffuseFactor"), mat.DiffuseFactor);
		dynamicMaterial->SetScalarParameterValue(FName("DepthOffset"), mat.DepthOffset);
		dynamicMaterial->SetScalarParameterValue(FName("Metallic"), mat.Metallic);
		dynamicMaterial->SetScalarParameterValue(FName("Roughness"), mat.Roughness);
		dynamicMaterial->SetScalarParameterValue(FName("Specular"), mat.Specular);
	}
	else
	{
		dynamicMaterial->SetScalarParameterValue(FName("Opacity"), mat.Opacity);
	}

	

	return dynamicMaterial;
}

void ARuntimeActor::SetMaterialSlot(int32 MaterialSlot, FName SlotName, UMaterialInterface* InMaterial)
{
	Provider->SetupMaterialSlot(MaterialSlot, SlotName, InMaterial);
}

void ARuntimeActor::SetActorName(FString& strName)
{
	ActorName = strName;
	if (ModelMesh != nullptr)
	{
		ModelMesh->MeshName = strName;
	}
}

ARuntimeActor* ARuntimeActor::GetRootActor()
{
	ARuntimeActor* rootActor = nullptr;
	traverseActor(rootActor);
	return rootActor;
}

void ARuntimeActor::BeginPlay()
{
	Super::BeginPlay();
}

void ARuntimeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
