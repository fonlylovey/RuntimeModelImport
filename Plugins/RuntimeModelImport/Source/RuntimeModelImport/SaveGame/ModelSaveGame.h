/**
 * Scene's actors, like RuntimeBaseActors
 */
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include <RuntimeMeshRenderable.h>
#include "ModelMesh.h"
#include "ModelMaterial.h"
#include "Templates/SharedPointer.h"
#include "UObject/UObjectGlobals.h"
#include "Tasks/Pipe.h"
#include "ModelSaveGame.generated.h"

//改动FModelMesh的话改的地方太多，所以保存的时候只能使用一个中间类FMeshData转换一下了
USTRUCT(BlueprintType)
struct FMeshData
{
	GENERATED_BODY()
public:
	FMeshData() = default;
	FMeshData(FModelMesh* pMesh);

	FModelMesh* ToModelMesh();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MeshID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MeshGUID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MeshName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ParentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRuntimeMeshSectionProperties> PropertyList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRuntimeMeshRenderableMeshData> RenderableList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FModelMaterial> MateritalList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<uint8> TextureList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform Transform;
};

UCLASS()
class UMemCalc : public UObject
{
	GENERATED_BODY()
public:
	UMemCalc(const FObjectInitializer& initializer)
		: Super(initializer)
	{
		
	}
	~UMemCalc() 
	{ 
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMeshData Data;
};

UCLASS()
class UModelSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	~UModelSaveGame()
	{
	}

	//序列化自己并保存到文件
	void SaveToFile();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32, FMeshData> MeshMap;

	//存档时使用的SP版本号
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ArchiveVersion;


	FString MeshUID;

	int ChunkID;

	FString FileDir;
private:
	static UE::Tasks::FPipe ModelSaveAsyncTaskPipe;
};
