#pragma once
#include "fbxsdk.h"
#include "CoreMinimal.h"
#include "ModelMesh.h"

class FBXMeshImport
{
public:
	FBXMeshImport();
	virtual ~FBXMeshImport();

	TSharedPtr<FModelMesh> LoadMesh(FbxScene* scene);

	inline TArray<TSharedPtr<FModelMesh>> GetMeshList() { return MeshList; };

	//inline TArray<TSharedPtr<FRuntimeMeshSectionData>> GetSections() { return SectionList; };
	
	TSharedPtr<FModelMesh> m_pRootMesh;

	int MeshNodeCount = 0; //带有Mesh的节点数量

private:
	void traverseNode(FbxNode* pNode, TSharedPtr<FModelMesh> pMesh);

	void readMesh(FbxNode* pNode, TSharedPtr<FModelMesh> pMesh);

	//读取顶点
	FVector3f readVertex(FbxVector4* pVertexArray, int vertexIndex);

	//读取法线
	FVector3f readNormal(FbxLayerElementNormal* pNormalArray, int vertexIndex, int meshVertexIndex);

	//读取顶点颜色
	FColor readColor(FbxLayerElementVertexColor* pColorArray, int vertexIndex, int meshVertexIndex);

	//读取纹理左边
	FVector2f readUV(FbxLayerElementUV* pUVArray, int vertexIndex, int meshTextureIndex, int layer);

	//读取切线
	FVector3f readTangent(FbxLayerElementTangent* pTangentArray, int vertexIndex, int meshVertexIndex);

	/*
	* readTransform函数和localToWorldMatrix函数
	* readTransform：如果以树形结构组织场景树的话，每个节点都拥有自己的变换矩阵
	* localToWorldMatrix：如果合并mesh之后，所有Mesh都是平行结构所以要转换成全局坐标
	*/
	//读取当前FBXNode的变换信息
	FTransform readTransform(FbxNode* pNode);

	//计算pNode局部到世界的变换矩阵
	FbxAMatrix localToWorldMatrix(FbxNode* pNode);

	//获取FbxNode所引用的材质IDs
	TArray<int32> readNodeMaterialIDs(FbxNode* pNode);

	FVector3f ConvertToRightUEPos(FbxVector4 Vector);

	FVector3f ConvertScale(FbxVector4 Vector);

	FQuat ConvertRotToQuat(FbxVector4 Vector);

	void loadSkeleton(FbxNode* pNode) { /*none*/ };

	void loadCamera(FbxNode* pNode) { /*none*/ };

	void loadLight(FbxNode* pNode) { /*none*/ };

	//根据传入的pNode拿到它下面子类型type的数量, 比如eMesh一共有多少个
	int32 GetRootNodeTypeNum(FbxNode* pNode, FbxNodeAttribute::EType emType, bool bRecursion = false);
private:
	int readIndex = 0; //当前处理的节点index
	int meshCount = 0;
	FbxAMatrix globalMatrix;
	//TArray<TSharedPtr<FRuntimeMeshSectionData>> SectionList;
	TArray<TSharedPtr<FModelMesh>> MeshList;
	TArray<int32> MatIndexArray;
};

