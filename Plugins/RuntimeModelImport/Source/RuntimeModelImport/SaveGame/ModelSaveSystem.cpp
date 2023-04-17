#include "ModelSaveSystem.h"
#include "Serialization/ArchiveCountMem.h"
#include "Serialization/BufferArchive.h"
#include <SaveGameSystem.h>
#include <PlatformFeatures.h>
#include <RMIDelegates.h>
#include <Serialization/ObjectAndNameAsStringProxyArchive.h>

#include "Serialization/LargeMemoryReader.h"

UE::Tasks::FPipe FModelSaveSystem::ModelSaveAsyncPipe { TEXT("ModelSaveAsyncPipe") };

FModelSaveSystem::FOnLoadCompleteDelegate FModelSaveSystem::OnLoadComplete;

void FModelSaveSystem::SaveToFile(const FString& proDir, FModelMesh* mesh)
{
	FString strMeshPath = proDir + mesh->MeshGUID;
	
	//获取项目文件夹下所有mesh文件
	IFileManager& fileMgr = IFileManager::Get();
	TArray<FString> meshFiles;
	fileMgr.FindFiles(meshFiles, *proDir, TEXT(".mesh"));

	//判断如果mesh文件已经在本地存在，直接返回
	/* 看业务需要，有可能已经存在也需要覆盖
	for (FString str : meshFiles)
	{
		if (str.Contains(mesh->MeshGUID))
		{
			return;
		}
	}*/

	ModelSaveAsyncPipe.Launch(UE_SOURCE_LOCATION, [this, strMeshPath, mesh]()
	{
			//计算内存大小，并且分块
			TMap<int, TArray<FMeshData>> ChunkMap;
			Traverse(mesh, ChunkMap);

			//将每一块内存数据保存的本地
			int index = 0;
			for (auto pair : ChunkMap)
			{
				UModelSaveGame* saveGame = NewObject<UModelSaveGame>();
				saveGame->AddToRoot();
				saveGame->ChunkID = pair.Key;
				saveGame->FileDir = strMeshPath;
				for (auto meshData : pair.Value)
				{
					saveGame->MeshMap.Add(meshData.MeshID, meshData);
				}
				Async(EAsyncExecution::TaskGraphMainThread, [=]()
					{
						FString strInfo = TEXT("正保存文件: ") + saveGame->MeshUID + TEXT("_") + FString::FormatAsNumber(saveGame->ChunkID);
						FRMIDelegates::OnImportProgressDelegate.Broadcast(1, index,
							ChunkMap.Num(), strInfo);
					});
				saveGame->SaveToFile();
				saveGame->RemoveFromRoot();
			}

			Async(EAsyncExecution::TaskGraphMainThread, [=]()
				{
					FString strInfo = TEXT("保存完成: ");
					FRMIDelegates::OnImportProgressDelegate.Broadcast(1, 2,
						2, strInfo);
				});
	});

}

void FModelSaveSystem::Traverse(FModelMesh* pMesh, TMap<int, TArray<FMeshData>>& chunkMap)
{
	FMeshData meshData = FMeshData(pMesh);
	static int chunkIndex = 0;
	static int meshIndex = 0;
	static int meshCount = pMesh->GetChildrenNum(true);
	static size_t dataSize = 1024 * 1024 * 10; //留10M的空余

	if (chunkMap.Num() == 0)
	{
		chunkIndex = 0;
		chunkMap.Add(0, TArray<FMeshData>());
	}
	UMemCalc* pMemCalc = NewObject<UMemCalc>();
	pMemCalc->Data = meshData;
	FArchiveCountMem memAr = FArchiveCountMem(pMemCalc);
	size_t meshSize = memAr.GetNum();
	if (meshSize > MAX_int32)
	{
		FString strInfo = meshData.MeshName + TEXT("所包含的数据大于2G，这部分数据将不会被保存！如果改模型合并过材质，请取消合并材质后再试。");
		return;
	}

	dataSize += meshSize;
	if (dataSize < MAX_int32)
	{
		TArray<FMeshData>& chunk = chunkMap[chunkIndex];
		chunk.Add(meshData);
	}
	else
	{
		dataSize = memAr.GetNum();
		chunkIndex++;
		chunkMap.Add(chunkIndex, { meshData });
	}

	Async(EAsyncExecution::TaskGraphMainThread, [=]()
		{
			FString strInfo = TEXT("正在计算内存: ") + FString::FormatAsNumber(dataSize);
			FRMIDelegates::OnImportProgressDelegate.Broadcast(1, meshIndex,
				meshCount, strInfo);
		});
	meshIndex++;
	for (auto val : pMesh->Children)
	{
		Traverse(val.Get(), chunkMap);
	}
}

TFuture<UModelSaveGame*> FModelSaveSystem::loadMeshFIleAsync(const FString& filePath, TFunction<void()> CompletionCallback)
{
	TFuture<UModelSaveGame*> futureObj = Async(EAsyncExecution::ThreadPool,
		[this, filePath]() { return LoadMeshFile(filePath);  }, CompletionCallback);
	return futureObj;
}


void FModelSaveSystem::LoadByFile(const FString& proDir)
{
	ModelSaveAsyncPipe.Launch(UE_SOURCE_LOCATION, [&, proDir]()
		{
			//获取项目文件夹下所有mesh文件
			IFileManager& fileMgr = IFileManager::Get();
			TArray<FString> meshFiles;
			fileMgr.FindFiles(meshFiles, *proDir, TEXT(".mesh"));

			//遍历所有mesh文件并且按照模型ID，读取所有文件到SaveGame对象中
			TSet<FString> idSet;

			TMultiMap<FString, UModelSaveGame*> saveGameMap;

			int index = 0;
			for (FString str : meshFiles)
			{
				Async(EAsyncExecution::TaskGraphMainThread, [=]()
					{
						FString strInfo = TEXT("正在加载模型... ");
						FRMIDelegates::OnImportProgressDelegate.Broadcast(1, index,
							meshFiles.Num(), strInfo);
					});
				//UModelSaveGame* savegame = loadMeshFIleAsync(proDir + str);
				int a = 0;
				
				TFuture<UModelSaveGame*> futureObj = loadMeshFIleAsync(proDir + str, [&]()
					{
						if (futureObj.IsValid())
						{
							FScopeLock ScopeLock(&Mutex);
							UModelSaveGame* savegame = futureObj.Get();
							check(savegame != nullptr);
							idSet.Add(savegame->MeshUID);
							saveGameMap.Add(savegame->MeshUID, savegame);
							index++;
						}
				});
				futureObj.Wait();
			}
			
			index = 0;
			//将多个文件块的数据合并成一个
			for (FString meshID : idSet)
			{
				Async(EAsyncExecution::TaskGraphMainThread, [=]()
					{
						FString strInfo = TEXT("正在创建Mesh... ");
						FRMIDelegates::OnImportProgressDelegate.Broadcast(1, index,
							idSet.Num(), strInfo);
					});
				TArray<UModelSaveGame*> saveList;
				saveGameMap.MultiFind(meshID, saveList);
				TMap<int32, FMeshData> DataMap;
				for (UModelSaveGame* saveObj : saveList)
				{
					FScopeLock ScopeLock(&Mutex);
					DataMap.Append(saveObj->MeshMap);
				}

				//将数据转为Mesh
				TMap<int32, TSharedPtr<FModelMesh>> ModelMeshMap;
				for (auto val : DataMap)
				{
					FScopeLock ScopeLock(&Mutex);
					FMeshData meshData = val.Value;
					FModelMesh* mesh = meshData.ToModelMesh();
					ModelMeshMap.Add(mesh->MeshID, MakeShareable(mesh));
				}

				//生成树形结构
				for (auto val : ModelMeshMap)
				{
					FScopeLock ScopeLock(&Mutex);
					auto TheMesh = val.Value;
					if (TheMesh->ParentID == -1)
					{
						TheMesh->IsRoot = true;
						MeshList.Add(TheMesh);
					}
					else
					{
						TSharedPtr<FModelMesh> Parent = ModelMeshMap[TheMesh->ParentID];
						Parent->Children.Add(TheMesh);
						TheMesh->Parent = Parent.Get();
					}
				}
				index++;
			}

			Async(EAsyncExecution::TaskGraphMainThread, [=]()
				{
					FString strInfo = TEXT("加载模型完成！");
					FRMIDelegates::OnImportProgressDelegate.Broadcast(1, 1,
						1, strInfo);
					OnLoadComplete.Broadcast(MeshList);
				});
			
			
		});
}

UModelSaveGame* FModelSaveSystem::LoadMeshFile(const FString& filePath)
{
	FString modelID = FPaths::GetBaseFilename(filePath);
	modelID = modelID.Left(modelID.Find("_"));
	UModelSaveGame* meshObj = NULL;
	TSharedRef<TArray<uint8>> ObjectBytes = MakeShared<TArray<uint8>>();
	bool bSuccess = FFileHelper::LoadFileToArray(ObjectBytes.Get(), *filePath);
	if (bSuccess)
	{
		FMemoryReader MemoryReader(ObjectBytes.Get(), true);

		FString SaveGameClassName;
		MemoryReader << SaveGameClassName;

		UClass* SaveGameClass = FindObject<UClass>(ANY_PACKAGE, *SaveGameClassName);
		if (SaveGameClass == NULL)
		{
			SaveGameClass = LoadObject<UClass>(NULL, *SaveGameClassName);
		}
		USaveGame* saveGame = NewObject<USaveGame>(GetTransientPackage(), SaveGameClass);
		if (saveGame != NULL)
		{
			//FLargeMemoryReader Ar(ObjectBytes->GetData(), ObjectBytes->Num());
			FObjectAndNameAsStringProxyArchive Ar(MemoryReader, true);
			saveGame->Serialize(Ar);
		}
		meshObj = Cast<UModelSaveGame>(saveGame);
		meshObj->MeshUID = modelID;
	}
	return meshObj;
}

