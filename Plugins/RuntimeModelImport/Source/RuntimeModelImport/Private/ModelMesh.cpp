#include "ModelMesh.h"
#include "Engine/StreamableManager.h"

FModelMesh::FModelMesh()
	: MeshID(-1)
	, MeshName("")
	, ParentID(-1)
	, Parent(nullptr)
	, IsRoot(false)
	, MeshMatrix(FVector::ZeroVector)
{
	Sections.Empty();
	Children.Empty();
	MaterialList.Empty();
}

FModelMesh::FModelMesh(int32 ID, FString strName)
	: MeshID(ID)
	, MeshName(strName)
	, ParentID(-1)
	, Parent(nullptr)
	, IsRoot(false)
	, MeshMatrix(FVector::ZeroVector)
{
	Sections.Empty();
	Children.Empty();
	MaterialList.Empty();
}

FModelMesh::~FModelMesh()
{
	MeshID = -1;
	MeshName = TEXT("");
	ParentID = -1;
	Parent = nullptr;
	IsRoot = false;
	MeshMatrix = FTransform::Identity;
	Children.Empty();
	MaterialList.Empty();
}

int32 FModelMesh::GetChildrenNum(bool bRecursion /*= false*/)
{
	int32 nRet = Children.Num();
	for (int i = 0; i < Children.Num(); ++i)
	{
		if (Children[i].Get())
		{
			nRet += Children[i].Get()->GetChildrenNum(bRecursion);
		}		
	}

	return nRet;
}
