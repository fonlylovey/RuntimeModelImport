#include "ModelOperator.h"
#include "OBJReader.h"
#include "IFCReader.h"
#include "FBXReader.h"
#include <RMITypes.h>
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "RMIDelegates.h"
#include "Engine/World.h"

FModelOperator* FModelOperator::s_pSelf = nullptr;

FModelOperator::FOnSpawnCompleteDelegate FModelOperator::OnSpawnComplete;

FModelOperator::FModelOperator()
{
	m_pSaveModelPtr = MakeShared<FModelSaveSystem>();
	//监听Mesh树状结构完成
	FRMIDelegates::OnMeshTreeBuildFinishDelegate.Clear();
	FRMIDelegates::OnMeshTreeBuildFinishDelegate.AddRaw(this, &FModelOperator::OnMeshTreeBuildFinishDelegateListen);
}

FModelOperator::~FModelOperator()
{
	m_pSaveModelPtr = nullptr;
}

ARuntimeActor* FModelOperator::ReadModelFile(const FString& strPath, const FImportOptions& options)
{
	m_pModelActor = nullptr;
	if(FPaths::FileExists(strPath))
	{
		m_option = options;
		m_pModelActor = GWorld->SpawnActor<ARuntimeActor>(ARuntimeActor::StaticClass(), FVector(0, 0, 0), FRotator(0, 0, 0));

		//创建一个新的模型读取者
		FString strSuffix = GetSuffix(strPath);
		m_pReader = CreateReader(strSuffix);
		FRMIDelegates::OnImportStartDelegate.Broadcast(strPath);
		m_pReader->ReadFile(strPath, options);
		return m_pModelActor;
	}
	return m_pModelActor;
}

void FModelOperator::SaveModel(const FString& saveDir, ARuntimeActor* actor)
{
	FModelMesh* modelMesh = actor->GetModelMesh();
	modelMesh->MeshMatrix = actor->GetActorTransform();
	m_pSaveModelPtr->SaveToFile(saveDir, modelMesh);
}

void FModelOperator::LoadModel(const FString& fileDir)
{
	auto loadComplete = [this](const TArray<TSharedPtr<FModelMesh>>& modelList)
	{
		ActorList.Empty();
		for (TSharedPtr<FModelMesh> meshPtr : modelList)
		{
			UWorld* GameWorld = GWorld;
			auto gEngine = GEngine;
			auto worldList = gEngine->GetWorldContexts();
			if(GameWorld->WorldType == EWorldType::Editor)
			{
				for (auto theWorld : worldList)
				{
					if(theWorld.WorldType != EWorldType::Editor)
					{
						GameWorld = theWorld.World();
					}
				}
			}
			check(GameWorld != nullptr);
			ARuntimeActor* actor = GameWorld->SpawnActorDeferred<ARuntimeActor>(ARuntimeActor::StaticClass(),
				meshPtr->MeshMatrix);
			actor->GUID = meshPtr->MeshGUID;
			actor->SetModelMesh(meshPtr);
			ActorList.Add(actor);
		}
		//在这里MeshPtr的引用计数已经增加，清空数组不会在析构MehsPtr，将控制权移交给Actor
		m_pSaveModelPtr->MeshList.Empty();
		OnSpawnComplete.Broadcast(ActorList);
	};
	
	m_pSaveModelPtr->OnLoadComplete.Clear();
	m_pSaveModelPtr->OnLoadComplete.AddLambda(loadComplete);
	m_pSaveModelPtr->LoadByFile(fileDir);
	
}

TSharedPtr<IReaderInterface> FModelOperator::CreateReader(const FString& fileSuffix)
{
	TSharedPtr<IReaderInterface> reader = nullptr;
	if (fileSuffix.Equals("fbx", ESearchCase::IgnoreCase))
	{
		reader = MakeShared<FBXReader>();
	}
	else if(fileSuffix.Equals("obj", ESearchCase::IgnoreCase))
	{
		reader = MakeShared<OBJReader>();
	}
	else if (fileSuffix.Equals("ifc", ESearchCase::IgnoreCase))
	{
		reader = MakeShared<IFCReader>();
	}
	
	return reader;
}

FString FModelOperator::GetSuffix(const FString& strPath)
{
	return FPaths::GetExtension(strPath);
}

FModelOperator* FModelOperator::Instance()
{
	if (s_pSelf == nullptr)
	{
		s_pSelf = new FModelOperator();
	}
	return s_pSelf;
}

void FModelOperator::OnMeshTreeBuildFinishDelegateListen(FModelMesh* pRoot)
{
	if (!pRoot)
		return;
	pRoot->MeshGUID = FGuid::NewGuid().ToString();
	m_pModelActor->SetModelMesh(MakeShareable(pRoot));
	m_pModelActor->GUID = pRoot->MeshGUID;
	FRMIDelegates::OnImportCompleteDelegate.Broadcast(m_pModelActor);
}