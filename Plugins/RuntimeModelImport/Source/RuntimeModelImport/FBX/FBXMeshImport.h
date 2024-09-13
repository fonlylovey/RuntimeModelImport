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

	int MeshNodeCount = 0; //����Mesh�Ľڵ�����

private:
	void traverseNode(FbxNode* pNode, TSharedPtr<FModelMesh> pMesh);

	void readMesh(FbxNode* pNode, TSharedPtr<FModelMesh> pMesh);

	//��ȡ����
	FVector3f readVertex(FbxVector4* pVertexArray, int vertexIndex);

	//��ȡ����
	FVector3f readNormal(FbxLayerElementNormal* pNormalArray, int vertexIndex, int meshVertexIndex);

	//��ȡ������ɫ
	FColor readColor(FbxLayerElementVertexColor* pColorArray, int vertexIndex, int meshVertexIndex);

	//��ȡ�������
	FVector2f readUV(FbxLayerElementUV* pUVArray, int vertexIndex, int meshTextureIndex, int layer);

	//��ȡ����
	FVector3f readTangent(FbxLayerElementTangent* pTangentArray, int vertexIndex, int meshVertexIndex);

	/*
	* readTransform������localToWorldMatrix����
	* readTransform����������νṹ��֯�������Ļ���ÿ���ڵ㶼ӵ���Լ��ı任����
	* localToWorldMatrix������ϲ�mesh֮������Mesh����ƽ�нṹ����Ҫת����ȫ������
	*/
	//��ȡ��ǰFBXNode�ı任��Ϣ
	FTransform readTransform(FbxNode* pNode);

	//����pNode�ֲ�������ı任����
	FbxAMatrix localToWorldMatrix(FbxNode* pNode);

	//��ȡFbxNode�����õĲ���IDs
	TArray<int32> readNodeMaterialIDs(FbxNode* pNode);

	FVector3f ConvertToRightUEPos(FbxVector4 Vector);

	FVector3f ConvertScale(FbxVector4 Vector);

	FQuat ConvertRotToQuat(FbxVector4 Vector);

	void loadSkeleton(FbxNode* pNode) { /*none*/ };

	void loadCamera(FbxNode* pNode) { /*none*/ };

	void loadLight(FbxNode* pNode) { /*none*/ };

	//���ݴ����pNode�õ�������������type������, ����eMeshһ���ж��ٸ�
	int32 GetRootNodeTypeNum(FbxNode* pNode, FbxNodeAttribute::EType emType, bool bRecursion = false);
private:
	int readIndex = 0; //��ǰ����Ľڵ�index
	int meshCount = 0;
	FbxAMatrix globalMatrix;
	//TArray<TSharedPtr<FRuntimeMeshSectionData>> SectionList;
	TArray<TSharedPtr<FModelMesh>> MeshList;
	TArray<int32> MatIndexArray;
};

