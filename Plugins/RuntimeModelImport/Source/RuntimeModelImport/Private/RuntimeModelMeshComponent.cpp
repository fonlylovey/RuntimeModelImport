#include "RuntimeModelMeshComponent.h"

#include "ModelMesh.h"

URuntimeModelMeshComponent::URuntimeModelMeshComponent()
{
	
}

URuntimeModelMeshComponent::~URuntimeModelMeshComponent()
{
}

void URuntimeModelMeshComponent::BeginDestroy()
{
	Super::BeginDestroy();
}

void URuntimeModelMeshComponent::CreateMesh(FModelMesh* mesh)
{
	UStaticMesh* pStaticMesh = GetStaticMesh();
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
	FBoxSphereBounds BoundingBoxAndSphere;
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
    	//pStaticMesh->AddMaterial();
    	for (int32 i = 0; i < section->Vertexes.Num(); i++)
    	{
    		FStaticMeshBuildVertex vertex;
    		vertex.Position = FVector3f(section->Vertexes[0]);
    		vertex.UVs[0] = section->TexCoord0[0];
    		vertex.TangentX = FVector3f(1, 0, 0);
    		vertex.TangentY = FVector3f(0, 1, 0);
    		vertex.TangentZ = FVector3f(0, 0, 1);
    		vertexData.Add(vertex);
    	}

    	TArray<uint32> indices;
    	for (int32 i = 0; i < section->Indexes.Num(); i++)
    	{
    		indices.Add( section->Indexes[i] + IndexFlag);
    	}
    	LODResources.IndexBuffer.AppendIndices(section->Indexes.GetData(), section->Indexes.Num());
    	sectionIndex = sectionIndex + indexCount;
    	IndexFlag = vertexData.Num();

    	BoundingBoxAndSphere = BoundingBoxAndSphere + section->BoundBox;
    }
    
    LODResources.VertexBuffers.PositionVertexBuffer.AppendVertices(vertexData.GetData(), vertexData.Num());
	LODResources.VertexBuffers.StaticMeshVertexBuffer.AppendVertices(vertexData.GetData(), vertexData.Num());
    
    //设置包围盒
	RenderData->Bounds = MoveTemp(BoundingBoxAndSphere);
    pStaticMesh->SetRenderData(MoveTemp(RenderData));

    pStaticMesh->InitResources();
    pStaticMesh->CalculateExtendedBounds();
}
