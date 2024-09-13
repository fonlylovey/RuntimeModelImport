#include "ModelOperator.h"

#include <functional>

#include "OBJ/OBJReader.h"
#include "IFC/IFCReader.h"
#include "FBX/FBXReader.h"
#include "STL/STLReader.h"
#include "RMIDelegates.h"
#include "Engine/World.h"

FModelOperator* FModelOperator::s_pSelf = nullptr;


FModelOperator::FModelOperator()
{
	if (!ThreadPool.IsValid())
	{
		ThreadPool = TUniquePtr<FQueuedThreadPool>(FQueuedThreadPool::Allocate());
		ThreadPool->Create(4,  64 * 1024, TPri_Normal, TEXT("RuntimeModelImport"));
	}
}

FModelOperator::~FModelOperator()
{
	
}
/*
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
	TSharedPtr<FModelMesh> modelMesh = actor->GetModelMesh();
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
*/
TSharedPtr<FReaderBase> FModelOperator::CreateReader(const FString& FilePath)
{
	FString fileSuffix = GetSuffix(FilePath);
	TSharedPtr<FReaderBase> reader = nullptr;
	if (fileSuffix.Equals("fbx", ESearchCase::IgnoreCase))
	{
		reader = MakeShared<FFBXReader>(FilePath);
	}
	else if(fileSuffix.Equals("obj", ESearchCase::IgnoreCase))
	{
		reader = MakeShared<FOBJReader>(FilePath);
	}
	else if (fileSuffix.Equals("ifc", ESearchCase::IgnoreCase))
	{
		reader = MakeShared<FIFCReader>(FilePath);
	}
	else if (fileSuffix.Equals("stl", ESearchCase::IgnoreCase))
	{
		reader = MakeShared<FSTLReader>(FilePath);
	}
	
	return reader;
}

FString FModelOperator::GetSuffix(const FString& FilePath)
{
	return FPaths::GetExtension(FilePath);
}

FQueuedThreadPool& FModelOperator::GetThreadPool()
{
	return *ThreadPool; 
}

FModelOperator* FModelOperator::Instance()
{
	if (s_pSelf == nullptr)
	{
		s_pSelf = new FModelOperator();
	}
	return s_pSelf;
}

