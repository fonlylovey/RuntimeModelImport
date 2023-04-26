#pragma once
#include "fbxsdk.h"
#include "CoreMinimal.h"
#include "ModelMesh.h"
#include "Runtime/Core/Public/HAL/ThreadingBase.h"
#include "IImageWrapperModule.h"

typedef TMap<int32, TSharedPtr<FModelMaterial>> MatMap;

class FBXMaterialImport : public FGCObject
{
public:
	FBXMaterialImport();
	virtual ~FBXMaterialImport();

	virtual void AddReferencedObjects(FReferenceCollector& Collector);

	void LoadMaterial(FbxScene* scene, const FString& strPath);

	MatMap MaterialMap;
	int MeshNodeCount = 0; //����Mesh�Ľڵ�����

private:
	//����ϵͳ��ȡ ���ʷ�Ϊ ��ͨ���ʡ�����shader������
	TSharedPtr<FModelMaterial> readMaterial(FbxSurfaceMaterial* pSurfaceMaterial);

	//���ﺯ���ڲ�ʹ���ƶ����壬��Ҫֱ�ӷ�����ʱ����
	bool readTexture(FString textureName, FbxSurfaceMaterial* pSurfaceMaterial, FModelTexture& byteData);

	bool loadImage(FString strPath, FModelTexture& byteData);

	float readFloatProperty(FString propertyName, FbxSurfaceMaterial* pSurfaceMaterial);
	FLinearColor readColorProperty(FString propertyName, FbxSurfaceMaterial* pSurfaceMaterial);

	void readNumberProperty(TSharedPtr<FModelMaterial> modelMat, FbxSurfaceMaterial* pSurfaceMaterial);

	void readShader(FbxSurfaceMaterial* pSurfaceMaterial, const FbxImplementation* pImplementation, FbxString& shaderType);

	//�ж��Ƿ�Ϊ͸������ �����͸����ֱ������͸���ĵ�ֵ�����򷵻�0
	float isTransparent(FbxSurfaceMaterial* pSurfaceMaterial);

	//�鿴�Ƿ�ʹ��Ӳ����ɫ
	const FbxImplementation* LoadImpl(FbxSurfaceMaterial* pMaterial);

	FLinearColor ConvertToColor(FbxProperty prop);

	float ConvertToFloat(FbxProperty prop);
	


private:
	int readIndex = 0; //��ǰ����Ľڵ�index
	//����ID+����ʵ��
	FString strFBxPath;
	FString strDefaultMat;
	FString strTransparentMat;
	UMaterialInterface* DefaultMat;
	UMaterialInterface* TransparentMat;
	FCriticalSection Mutex;
	IImageWrapperModule* ImageModule;
	//FReferenceCollector* GCCollector;
};

