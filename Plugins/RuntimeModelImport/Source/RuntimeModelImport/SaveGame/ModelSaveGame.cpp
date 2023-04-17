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
	TArray<uint8> ObjectBytes;
	FMemoryWriter MemoryWriter(ObjectBytes, true);

	FString SaveGameClassName = GetClass()->GetName();
	MemoryWriter << SaveGameClassName;

	FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, false);
	Serialize(Ar);
	FString strFilePath = FileDir + "_" + FString::FromInt(ChunkID) + TEXT(".mesh");
	FFileHelper::SaveArrayToFile(ObjectBytes, *strFilePath);
}

FMeshData::FMeshData(FModelMesh* pMesh)
{
	MeshID = pMesh->MeshID;
	MeshGUID = pMesh->MeshGUID;
	MeshName = pMesh->MeshName;
	ParentID = pMesh->ParentID;
	Transform = pMesh->MeshMatrix;
	for (auto section : pMesh->SectionList)
	{
		PropertyList.Add(section->Properties);
		RenderableList.Add(section->MeshData);
	}
	
	MateritalList = pMesh->MaterialList;

}

FModelMesh* FMeshData::ToModelMesh()
{
	FModelMesh* modelMesh = new FModelMesh();
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
		modelMesh->MaterialList = MateritalList;
	}
	return modelMesh;
}
