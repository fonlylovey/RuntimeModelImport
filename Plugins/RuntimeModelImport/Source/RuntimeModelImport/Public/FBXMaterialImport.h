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
	int MeshNodeCount = 0; //带有Mesh的节点数量

private:
	//材质系统读取 材质分为 普通材质、纹理、shader三部分
	TSharedPtr<FModelMaterial> readMaterial(FbxSurfaceMaterial* pSurfaceMaterial);

	//这里函数内部使用移动语义，不要直接返回临时对象
	bool readTexture(FString textureName, FbxSurfaceMaterial* pSurfaceMaterial, FModelTexture& byteData);

	bool loadImage(FString strPath, FModelTexture& byteData);

	float readFloatProperty(FString propertyName, FbxSurfaceMaterial* pSurfaceMaterial);
	FLinearColor readColorProperty(FString propertyName, FbxSurfaceMaterial* pSurfaceMaterial);

	void readNumberProperty(TSharedPtr<FModelMaterial> modelMat, FbxSurfaceMaterial* pSurfaceMaterial);

	void readShader(FbxSurfaceMaterial* pSurfaceMaterial, const FbxImplementation* pImplementation, FbxString& shaderType);

	//判断是否为透明材质 如果是透明的直径返回透明的的值，否则返回0
	float isTransparent(FbxSurfaceMaterial* pSurfaceMaterial);

	//查看是否使用硬件着色
	const FbxImplementation* LoadImpl(FbxSurfaceMaterial* pMaterial);

	FLinearColor ConvertToColor(FbxProperty prop);

	float ConvertToFloat(FbxProperty prop);
	


private:
	int readIndex = 0; //当前处理的节点index
	//材质ID+材质实例
	FString strFBxPath;
	FString strDefaultMat;
	FString strTransparentMat;
	UMaterialInterface* DefaultMat;
	UMaterialInterface* TransparentMat;
	FCriticalSection Mutex;
	IImageWrapperModule* ImageModule;
	//FReferenceCollector* GCCollector;
};

