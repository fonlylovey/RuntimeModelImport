#include "FBXReader.h"
#include <fbxsdk/scene/geometry/fbxlayer.h>
#include <string>
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "FBX/2020.2/include/fbxsdk/core/fbxsystemunit.h"
#include "RuntimeMeshRenderable.h"
#include <RMIDelegates.h>
#include "ModelOperator.h"
#include <assert.h>

FBXReader::FBXReader()
	: m_strModelPath(FString(TEXT("")))
	, SdkManager(nullptr)
	, m_pFbxScene(nullptr)
	, m_ImportOption(FImportOptions())
	, m_pModelMesh(nullptr)
	, m_pMeshImport(MakeShared<FBXMeshImport>())
	, m_pMaterialImport(MakeShared<FBXMaterialImport>())
{
}

FBXReader::~FBXReader()
{
	if (SdkManager)
	{
		if (SdkManager->GetIOSettings())
			SdkManager->GetIOSettings()->Destroy();

		SdkManager->Destroy();
		SdkManager = nullptr;
	}

	m_pMeshImport.Reset();
	m_pMeshImport = nullptr;

	m_pMaterialImport.Reset();
	m_pMaterialImport = nullptr;
}

void FBXReader::initFBXSDK()
{
	if (!SdkManager)
	{
		// Create the SdkManager
		SdkManager = FbxManager::Create();
		// create an IOSettings object
		FbxIOSettings* ios = FbxIOSettings::Create(SdkManager, IOSROOT);
		SdkManager->SetIOSettings(ios);
	}
	FbxImporter* fbxImporter = FbxImporter::Create(SdkManager, "");
	bool FileStatus = fbxImporter->Initialize(TCHAR_TO_UTF8(*m_strModelPath));
	if (FileStatus && fbxImporter->IsFBX())
	{
		m_pFbxScene = FbxScene::Create(SdkManager, "");
		//这个Import操作需要比较唱的世界，将FBX文件从硬盘读到内存当中
		fbxImporter->SetProgressCallback(&FbxImportCallback);
		FileStatus = fbxImporter->Import(m_pFbxScene);
		
		FbxString strError = fbxImporter->GetStatus().GetErrorString();
		char* charError;
		FbxUTF8ToAnsi(strError, charError);

		FbxIOFileHeaderInfo* headerInfo = fbxImporter->GetFileHeaderInfo();
		if (!FileStatus)
		{
			m_pFbxScene = nullptr;
			
			fbxImporter->Destroy();
			fbxImporter = nullptr;
			throw std::exception(charError);
		}
		fbxImporter->Destroy();
		fbxImporter = nullptr;
	}
	else
	{
		char* charError;
		FbxUTF8ToAnsi(fbxImporter->GetStatus().GetErrorString(), charError);
		throw std::exception(charError);
	}
}

FModelMesh* FBXReader::ReadFile(const FString& strPath)
{
	return ReadFile(strPath, FImportOptions());
}

FModelMesh* FBXReader::ReadFile(const FString& strPath, const FImportOptions& options)
{
	m_ImportOption = options;
	m_strModelPath = strPath;
	FString strFileName = GetFileName();

	//初始化FBXSDK，导入fbxscene
	try
	{
		m_initTask = Async(EAsyncExecution::TaskGraph, [=]()
			{
				initFBXSDK();
				//进行场景转换
				ConvertScene(m_pFbxScene);
			});
	}
	catch (std::exception ex)
	{
		throw ex;
	}
	
	//读取材质
	m_readMatTask = Async(EAsyncExecution::TaskGraph, [=]()
		{
			m_initTask.Wait();
			m_pMaterialImport->LoadMaterial(m_pFbxScene, strFileName);
		});

	//读取Mesh
	m_readMeshTask = Async(EAsyncExecution::TaskGraph, [=]()
		{
			m_readMatTask.Wait();
			m_pModelMesh = m_pMeshImport->LoadMesh(m_pFbxScene);
		});

	//关联材质
	MatMap matMap = m_pMaterialImport->GetMaterialMap();
	m_mergeMatTask = Async(EAsyncExecution::TaskGraph, [=]()
		{
			m_readMeshTask.Wait();
			m_pMaterialImport->MeshNodeCount = m_pMeshImport->MeshNodeCount;
			PairMaterial();
			//广播处理完全的完整mesh
			Async(EAsyncExecution::TaskGraphMainThread, [&]()
				{
					m_mergeMatTask.Wait();
					assert(m_pModelMesh != nullptr);
					m_pModelMesh->MeshName = GetFileName();
					FRMIDelegates::OnMeshTreeBuildFinishDelegate.Broadcast(m_pModelMesh);
				});
		});

	
	return nullptr;
}

void FBXReader::PairMaterial()
{
	flag = 0;
	/**
	 * 将材质附加到Mesh上  目前只有一种按材质优化的方式
	 */
	 //按材质合并Section
	if (m_ImportOption.MergeByMaterial)
	{
		LinkAndMergeByMaterial();
	}
	else 
	{
		LinkMaterial(m_pModelMesh);
	}
}

void FBXReader::LinkMaterial(FModelMesh* pMesh)
{
	if (m_pMaterialImport.IsValid())
	{
		int slotIndex = 0;
		MatMap matMap = m_pMaterialImport->GetMaterialMap();
		for (auto section : pMesh->SectionList)
		{
			int32 matID = section->Properties.MaterialSlot;
			FModelMaterial* mat = matMap.Find(matID);
			if (mat != nullptr)
			{
				section->Properties.MaterialSlot = slotIndex;
				pMesh->MaterialList.Add(*mat);
				slotIndex++;

				flag++;
				FString strMatName = mat->Name;
				//进度广播
				Async(EAsyncExecution::TaskGraphMainThread, [=]()
					{
						FString strInfo = TEXT("关联材质: ") + strMatName;
						FRMIDelegates::OnImportProgressDelegate.Broadcast(1, flag,
							m_pMaterialImport->MeshNodeCount, strInfo);
					});

			}

		}

		for (auto cMesh : pMesh->Children)
		{
			LinkMaterial(cMesh.Get());
		}
	}
}

void FBXReader::LinkAndMergeByMaterial()
{
	m_pModelMesh->Children.Empty();
	TMap<int32, TSharedPtr<FModelMesh, ESPMode::ThreadSafe>> MatMeshMap;
	TArray<TSharedPtr<FRuntimeMeshSectionData>> meshMatsNap = m_pMeshImport->GetSections();
	auto matMap = m_pMaterialImport->GetMaterialMap();
	int sectionCount = meshMatsNap.Num();
	bool isChunk = false;
	int index = 0;
	for (auto section : meshMatsNap)
	{
		FString strMatName = "";
		int32 matID = section->Properties.MaterialSlot;
		TSharedPtr<FModelMesh, ESPMode::ThreadSafe> matMesh = nullptr;
		if (MatMeshMap.Contains(matID))
		{
			matMesh = MatMeshMap[matID];
		}
		else
		{
			matMesh = MakeShared<FModelMesh, ESPMode::ThreadSafe>();
			MatMeshMap.Add(matID, matMesh);
		}
		section->Properties.MaterialSlot = 0;
		FModelMaterial* matPtr = matMap.Find(matID);
		if (matMesh->SectionList.Num() == 0)
		{
			if (matPtr != nullptr)
			{
				matMesh->MeshID = matID;
				matMesh->ParentID = m_pModelMesh->MeshID;
				matMesh->Parent = m_pModelMesh;
				matMesh->MeshName = matPtr->Name;
				matMesh->SectionList.Add(section);
				matMesh->MaterialList.Add(*matPtr);
				m_pModelMesh->Children.Add(matMesh);
				strMatName = matPtr->Name;
			}
		}
		else
		{
			//合并section
			FRuntimeMeshRenderableMeshData& renderDataA = matMesh->SectionList[0]->MeshData;
			FRuntimeMeshRenderableMeshData renderDataB = section->MeshData;
			size_t newSize = renderDataA.Positions.Num() + renderDataB.Positions.Num();
			if (newSize > 40000000)
			{
				int oldDef = matID << index;
				MatMeshMap.Add(oldDef, matMesh);
				matMesh = MakeShared<FModelMesh, ESPMode::ThreadSafe>();
				if (matPtr != nullptr)
				{
					matMesh->MaterialList.Add(*matPtr);
					matMesh->MeshID = matID;
					matMesh->ParentID = m_pModelMesh->MeshID;
					matMesh->Parent = m_pModelMesh;
					matMesh->MeshName = matPtr->Name;
					m_pModelMesh->Children.Add(matMesh);
					strMatName = matPtr->Name;
				}
				MatMeshMap[matID] = matMesh;
				matMesh->SectionList.Add(section);
				index++;
				continue;
			}
			renderDataA.Positions.Append(renderDataB.Positions);
			
			int triIndex = 0;
			while (triIndex < renderDataB.Triangles.Num())
			{
				renderDataA.Triangles.Add(renderDataA.Triangles.Num());
				triIndex++;
			}

			renderDataA.Colors.Append(renderDataB.Colors);
			renderDataA.TexCoords.Append(renderDataB.TexCoords);
			renderDataA.Tangents.Append(renderDataB.Tangents);
		}

		//进度广播
		Async(EAsyncExecution::TaskGraphMainThread, [=]()
			{
				FString strInfo = TEXT("关联材质: " + strMatName);
				FRMIDelegates::OnImportProgressDelegate.Broadcast(1, index, sectionCount, strInfo);
			});
		index++;
	}
}

bool FBXReader::FbxImportCallback(void* pArgs, float pPercentage, const char* pStatus)
{
	FString strInfo = TEXT("加载模型文件...");

	Async(EAsyncExecution::TaskGraphMainThread, [=]()
		{
			FRMIDelegates::OnImportProgressDelegate.Broadcast(1, pPercentage, 100, strInfo);
		});
	
	return true;
}

void FBXReader::ConvertScene(FbxScene* pScene)
{
	if (pScene)
	{
		FbxAxisSystem::ECoordSystem CoordSystem = FbxAxisSystem::eRightHanded;
		FbxAxisSystem::EUpVector UpVector = FbxAxisSystem::eZAxis;
		FbxAxisSystem::EFrontVector FrontVector = FbxAxisSystem::eParityEven;

		FbxAxisSystem UnrealImportAxis(UpVector, FrontVector, CoordSystem);

		FbxAxisSystem SourceSetup = pScene->GetGlobalSettings().GetAxisSystem();

		if (SourceSetup != UnrealImportAxis)
		{
			FbxRootNodeUtility::RemoveAllFbxRoots(pScene);
			UnrealImportAxis.ConvertScene(pScene);
		}

		//模型单位化成虚幻的厘米
		if (pScene->GetGlobalSettings().GetSystemUnit() != FbxSystemUnit::cm)
		{
			FbxSystemUnit::cm.ConvertScene(pScene);
		}

		pScene->GetAnimationEvaluator()->Reset();
	}
}

FString FBXReader::GetFileName()
{
	
	return FPaths::GetPath(m_strModelPath) + "\\" + FPaths::GetBaseFilename(m_strModelPath);
}
