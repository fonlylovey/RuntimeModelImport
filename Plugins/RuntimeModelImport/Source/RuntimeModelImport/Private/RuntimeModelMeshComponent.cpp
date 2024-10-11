#include "RuntimeModelMeshComponent.h"

#include "ModelMesh.h"

URuntimeModelMeshComponent::URuntimeModelMeshComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bUseComplexAsSimpleCollision = true;
}

URuntimeModelMeshComponent::~URuntimeModelMeshComponent()
{
}

void URuntimeModelMeshComponent::BeginDestroy()
{
	Super::BeginDestroy();
}

void URuntimeModelMeshComponent::CreateMesh(TSharedPtr<FModelMesh> mesh)
{
	int32 sectionIndex = 0;

    TArray<FVector> normals;
    TArray<FProcMeshTangent> tangents;
    TArray<FVector2D> uvs;
    TArray<FLinearColor> vertexColors;
    vertexColors.Add(FLinearColor(1, 1, 1, 1));

	for (const auto& section : mesh->Sections)
	{
		CreateMeshSection_LinearColor(sectionIndex, section->Vertexes, section->Indexes, section->Normals, section->TexCoord0, vertexColors, tangents, false);
    }
}

/*
void URuntimeModelMeshComponent::CreateMesh(TSharedPtr<FModelMesh> mesh)
{
	UStaticMesh* pStaticMesh = GetStaticMesh();
	if (pStaticMesh != nullptr)
	{
		pStaticMesh->ReleaseResources();
		pStaticMesh->BeginDestroy();
	}
	pStaticMesh = NewObject<UStaticMesh>();
	SetStaticMesh(pStaticMesh);
	pStaticMesh->NeverStream = true;
	pStaticMesh->SetFlags(RF_Transient | RF_DuplicateTransient | RF_TextExportTransient);
	TUniquePtr<FStaticMeshRenderData> RenderData = MakeUnique<FStaticMeshRenderData>();
	RenderData->AllocateLODResources(1);
	pStaticMesh->CreateBodySetup();
	FStaticMeshLODResources& LODResources = RenderData->LODResources[0];
	LODResources.bHasColorVertexData = false;
	LODResources.bHasDepthOnlyIndices = false;
	LODResources.bHasReversedIndices = false;
	LODResources.bHasReversedDepthOnlyIndices = false;

	int32 sectionIndex = 0;
	uint32 IndexFlag = 0;
	TArray<FStaticMeshBuildVertex> vertexData;

	FStaticMeshSectionArray& sectionArray = LODResources.Sections;
    FBox BoundingBox;
    BoundingBox.Init();
    for (const auto& section : mesh->Sections)
    {
    	int32 indexCount = section->Indexes.Num();
    	FStaticMeshSection& staticSection = sectionArray.AddDefaulted_GetRef();
    	staticSection.bEnableCollision = true;
    	staticSection.MaterialIndex = 0;
    	staticSection.NumTriangles = indexCount / 3;
    	staticSection.FirstIndex = sectionIndex;
    	staticSection.MinVertexIndex = sectionIndex;
    	staticSection.MaxVertexIndex = sectionIndex + indexCount - 1;
    	pStaticMesh->AddMaterial(GetMaterial(0));
    	for (int32 i = 0; i < section->Vertexes.Num(); i++)
    	{
    		FStaticMeshBuildVertex vertex;
    		vertex.Position = FVector3f(section->Vertexes[i] * 10);
    		vertex.UVs[0] = section->TexCoord0[i];
    		vertex.TangentX = FVector3f(1, 0, 0);
    		vertex.TangentY = FVector3f(0, 1, 0);
    		vertex.TangentZ = FVector3f(0, 0, 1);
    		vertexData.Add(vertex);
            BoundingBox += FVector(section->Vertexes[i] * 10);
    	}

    	TArray<uint32> indices;
    	for (int32 i = 0; i < section->Indexes.Num(); i++)
    	{
    		indices.Add( section->Indexes[i] + IndexFlag);
    	}
    	LODResources.IndexBuffer.AppendIndices(section->Indexes.GetData(), section->Indexes.Num());
    	sectionIndex = sectionIndex + indexCount;
    	IndexFlag = vertexData.Num();

    	
    }
    
    LODResources.VertexBuffers.PositionVertexBuffer.AppendVertices(vertexData.GetData(), vertexData.Num());
	LODResources.VertexBuffers.StaticMeshVertexBuffer.AppendVertices(vertexData.GetData(), vertexData.Num());
   
    //设置包围盒
    FBoxSphereBounds BoundingBoxAndSphere;
    BoundingBoxAndSphere.Origin = FVector(BoundingBox.GetCenter());
    BoundingBoxAndSphere.BoxExtent = FVector(BoundingBox.GetExtent());
    BoundingBoxAndSphere.SphereRadius = BoundingBoxAndSphere.BoxExtent.Size();
    RenderData->Bounds = MoveTemp(BoundingBoxAndSphere);
    pStaticMesh->SetRenderData(MoveTemp(RenderData));

    pStaticMesh->InitResources();
    pStaticMesh->CalculateExtendedBounds();
}
*/