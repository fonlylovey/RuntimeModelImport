#include "ModelSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "RuntimeActor.h"
#include <Engine/GameInstance.h>
#include "HAL/FileManager.h"
#include <Serialization/ObjectAndNameAsStringProxyArchive.h>
#include "Serialization/LargeMemoryWriter.h"

FArchive& operator<<(FArchive& Ar, FMeshData& material)
{
	Ar << material.MeshID;
	return Ar;
}

void UModelSaveGame::SaveToFile()
{
	TArray64<uint8> ObjectBytes;
	//FMemoryWriter MemoryWriter(ObjectBytes, true);
	FLargeMemoryWriter Ar;
	FString SaveGameClassName = GetClass()->GetName();
	Ar << SaveGameClassName;

	//FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, false);
	Serialize(Ar);
	uint8* ArchiveData = Ar.GetData();
	int64 ArchiveSize = Ar.TotalSize();
	TArray64<uint8> Buffer(ArchiveData, ArchiveSize);
	FString strFilePath = FileDir + "_" + FString::FromInt(ChunkID) + TEXT(".mesh");
	FFileHelper::SaveArrayToFile(Buffer, *strFilePath);
}

FMeshData::FMeshData(FModelMesh* pMesh)
{
	MeshID = pMesh->MeshID;
	MeshGUID = pMesh->MeshGUID;
	MeshName = pMesh->MeshName;
	ParentID = pMesh->ParentID;
	Transform = pMesh->MeshMatrix;
	for (TSharedPtr<FRuntimeMeshSectionData> section : pMesh->SectionList)
	{
		PropertyList.Add(section->Properties);
		RenderableList.Add(section->MeshData);
	}

	for (TSharedPtr<FModelMaterial> matPtr : pMesh->MaterialList)
	{
		MateritalList.Add(*matPtr);
	}
}

FMeshData::~FMeshData()
{
	PropertyList.Empty();
	RenderableList.Empty();
	MateritalList.Empty();
}

TSharedPtr<FModelMesh> FMeshData::ToModelMesh()
{
	TSharedPtr<FModelMesh> modelMesh = MakeShared<FModelMesh>();
	modelMesh->MeshID = MeshID;
	modelMesh->MeshGUID = MeshGUID;
	modelMesh->MeshName = MeshName;
	modelMesh->ParentID = ParentID;
	modelMesh->MeshMatrix = Transform;
	int count = PropertyList.Num();
	for (int i = 0; i < count; i++)
	{
		FRuntimeMeshSectionData section;
		section.MeshData = RenderableList[i];
		section.Properties = PropertyList[i];
		modelMesh->SectionList.Add(MakeShared<FRuntimeMeshSectionData>(section));
	}

	for (FModelMaterial MatRef : MateritalList)
	{
		modelMesh->MaterialList.Add(MakeShared<FModelMaterial>(MatRef));
	}

	return modelMesh;
}
