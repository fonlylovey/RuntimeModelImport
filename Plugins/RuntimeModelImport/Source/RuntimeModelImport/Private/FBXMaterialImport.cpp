#include "FBXMaterialImport.h"
#include "IImageWrapper.h"
#include "../Public/RMIDelegates.h"

FBXMaterialImport::FBXMaterialImport()
{
	MaterialMap.Reset();
	strDefaultMat = TEXT("/RuntimeModelImport/Materials/FBXMaterial.FBXMaterial");
	strTransparentMat = TEXT("/RuntimeModelImport/Materials/FBXTransparent.FBXTransparent");

	DefaultMat = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *strDefaultMat));
	TransparentMat = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *strTransparentMat));
	//GCCollector = nullptr;

	ImageModule = &FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
}

FBXMaterialImport::~FBXMaterialImport()
{
	MaterialMap.Empty();
}


void FBXMaterialImport::AddReferencedObjects(FReferenceCollector& Collector)
{
	//Collector.AddReferencedObject(DefaultMat);
	//Collector.AddReferencedObject(TransparentMat);
}

const MatMap FBXMaterialImport::LoadMaterial(FbxScene* scene, const FString& strPath)
{
	strFBxPath = strPath;
	scene->GetMaterialCount();
	FbxArray<FbxSurfaceMaterial*> matArray = FbxArray<FbxSurfaceMaterial*>();
	scene->FillMaterialArray(matArray);

	MaterialMap.Add(-1, FModelMaterial(-1, "Default Material"));
	int matSize = matArray.GetCount();
	for (int i = 0; i < matSize; i++)
	{
		FbxSurfaceMaterial* fbxMat = matArray[i];
		FString strMatName = UTF8_TO_TCHAR(fbxMat->GetName());
		FModelMaterial rModelMat = readMaterial(fbxMat);
		MaterialMap.Add(rModelMat.ID, rModelMat);

		FString strInfo = TEXT("创建材质：") + strMatName + "...";

		Async(EAsyncExecution::TaskGraphMainThread, [=]()
			{
				FRMIDelegates::OnImportProgressDelegate.Broadcast(1, i, matSize, strInfo);
			});
	} 

	return MaterialMap;
}

FModelMaterial FBXMaterialImport::readMaterial(FbxSurfaceMaterial* pSurfaceMaterial)
{
	//读取材质属性
	int32 matUniqueID = pSurfaceMaterial->GetUniqueID();
	FString strMatName = UTF8_TO_TCHAR(pSurfaceMaterial->GetName());
	FModelMaterial runtimeMat = FModelMaterial(matUniqueID, strMatName);
	//判断是不是透明材质
	runtimeMat.Opacity = isTransparent(pSurfaceMaterial);
	runtimeMat.IsTransparent = runtimeMat.Opacity == 1.0 ? false : true;

	static float depOffset = 0.01;
	runtimeMat.DepthOffset = 0.01;
	depOffset += 0.01;
	readNumberProperty(runtimeMat, pSurfaceMaterial);

	//因为异步和saveGame的原因，不要在sdk层读取和创建纹理，材质，直接读图片的byte数据，在到actor中创建

	readTexture(TEXT("DiffuseColor"), pSurfaceMaterial, runtimeMat.DiffuseMap);
	//Read贴图
	readTexture(TEXT("NormalMap"), pSurfaceMaterial, runtimeMat.NormalMap);

	FbxString shaderType = "HLSL";
	const FbxImplementation* pImplementation = GetImplementation(pSurfaceMaterial, FBXSDK_IMPLEMENTATION_HLSL);
	if (pImplementation == nullptr)
	{
		pImplementation = GetImplementation(pSurfaceMaterial, FBXSDK_IMPLEMENTATION_CGFX);
		shaderType = "CGFX";
	}
	if (pImplementation != nullptr)
	{
		readShader(pSurfaceMaterial, pImplementation, shaderType);
	}
	return runtimeMat;
}

bool FBXMaterialImport::readTexture(FString textureName, FbxSurfaceMaterial* pSurfaceMaterial, FModelTexture& byteData)
{
	//主要是从fbx中读取图片的路径和格式
	FbxProperty Prop = pSurfaceMaterial->FindProperty(TCHAR_TO_UTF8(*textureName));
	int texCOunt = Prop.GetSrcObjectCount<FbxTexture>();
	if (texCOunt > 0)
	{
		FbxTexture* texptr = Prop.GetSrcObject<FbxTexture>(0);
		FbxFileTexture* FbxTex = FbxCast<FbxFileTexture>(texptr);
		if (FbxTex == nullptr) return false;

		FString texName = UTF8_TO_TCHAR(FbxTex->GetName());
		if (texName.IsEmpty())
		{
			return false;
		}

		FString absPath = UTF8_TO_TCHAR(FbxTex->GetFileName());
		texName = FPaths::GetCleanFilename(absPath);
		FString relPath = UTF8_TO_TCHAR(FbxTex->GetRelativeFileName());
		bool isEx = FPaths::FileExists(relPath);
		FString texterePath = FPaths::FileExists(absPath) 
			? absPath : FPaths::FileExists(relPath) 
			? relPath : strFBxPath + ".fbm/" + texName;

		FString TextureName = textureName;
		TextureName.RemoveFromEnd(TEXT("Color"));	// DiffuseColor -> Diffuse
		TextureName.RemoveFromEnd(TEXT("Map"));	    // NormalMap -> Normal
		TextureName = TextureName + TEXT("Texture");	// Diffuse -> DiffuseTexture

		return loadImage(texterePath, byteData);
	}
	return false;
}

bool FBXMaterialImport::loadImage(FString strPath, FModelTexture& byteData)
{
	FString Extension = FPaths::GetExtension(strPath);
	EImageFormat ImageFormat = EImageFormat::Invalid;
	
	if (strPath.EndsWith(".png"))
	{
		ImageFormat = EImageFormat::PNG;
	}
	else if (strPath.EndsWith(".jpg") || strPath.EndsWith(".jpeg"))
	{
		ImageFormat = EImageFormat::JPEG;
	}
	else if (strPath.EndsWith(".bmp"))
	{
		ImageFormat = EImageFormat::BMP;
	}
	else if (strPath.EndsWith(".ico"))
	{
		ImageFormat = EImageFormat::ICO;

	}
	else if (strPath.EndsWith("exr"))
	{
		ImageFormat = EImageFormat::EXR;
	}
	else if (strPath.EndsWith(".icns"))
	{
		ImageFormat = EImageFormat::ICNS;
	}
	else if (strPath.EndsWith(".tga"))
	{
		ImageFormat = EImageFormat::TGA;
	}
	else if (strPath.EndsWith(".tif") || strPath.EndsWith(".tiff"))
	{
		ImageFormat = EImageFormat::TIFF;
	}

	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, *strPath))
	{
		return false;
	}

	
	TSharedPtr<IImageWrapper> ImageWrapper = ImageModule->CreateImageWrapper(ImageFormat);
	
	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
	{
		int width = ImageWrapper->GetWidth();
		int height = ImageWrapper->GetHeight();
		TArray<uint8> UncompressedBGRA;
		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
		{
			byteData.BulkData = UncompressedBGRA;
			byteData.SizeX = width;
			byteData.SizeY = height;
			return true;;
		}
		return false;
	}

	return false;
}


float FBXMaterialImport::readFloatProperty(FString propertyName, FbxSurfaceMaterial* pSurfaceMaterial)
{
	float Value = 0.5;
	FbxProperty Prop = pSurfaceMaterial->FindProperty(TCHAR_TO_UTF8(*propertyName));
	bool isOK = FbxProperty::HasDefaultValue(Prop);
	
	switch (Prop.GetPropertyDataType().GetType())
	{
	case eFbxFloat:
	{
		Value = Prop.Get<FbxFloat>();
		break;
	}
	case eFbxDouble:
	{
		Value = Prop.Get<FbxDouble>();
		break;
	}
	default:
		Value = 0.5;
	}
	return Value;
}

FLinearColor FBXMaterialImport::readColorProperty(FString propertyName, FbxSurfaceMaterial* pSurfaceMaterial)
{
	FLinearColor color;
	
	FbxPropertyT<FbxDouble3> lKFbxDouble3;
	FbxPropertyT<FbxDouble> lKFbxDouble1;
	FbxColor theColor;
	
	FbxProperty Prop = pSurfaceMaterial->FindProperty(TCHAR_TO_UTF8(*propertyName));
	bool isOK = FbxProperty::HasDefaultValue(Prop);

	
	switch (Prop.GetPropertyDataType().GetType())
	{
	case eFbxDouble3:
	{
		FbxDouble3 v = Prop.Get<FbxDouble3>();
		color.R = v.mData[0];
		color.G = v.mData[1];
		color.B = v.mData[2];
		color.A = 0.0f;
		break;
	}
	case eFbxDouble4:
	{
		FbxDouble4 v = Prop.Get<FbxDouble4>();
		color.R = v.mData[0];
		color.G = v.mData[1];
		color.B = v.mData[2];
		color.A = v.mData[3];
		break;
	}
	default:
		color;
	}
	
	return color;
}

void FBXMaterialImport::readNumberProperty(FModelMaterial& modelMat, FbxSurfaceMaterial* pSurfaceMaterial)
{
	FbxProperty prop = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
	modelMat.DiffuseColor = ConvertToColor(prop);
	prop = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuseFactor);
	modelMat.DiffuseFactor = ConvertToFloat(prop);
	prop = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sEmissive);
	modelMat.EmissiveColor = ConvertToColor(prop);
	prop = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sEmissiveFactor);
	modelMat.EmissiveFactor = ConvertToFloat(prop);
}

void FBXMaterialImport::readShader(FbxSurfaceMaterial* pSurfaceMaterial, const FbxImplementation* pImplementation, FbxString& shaderType)
{
	const FbxBindingTable* lRootTable = pImplementation->GetRootTable();
	FbxString lFileName = lRootTable->DescAbsoluteURL.Get();
	FbxString lTechniqueName = lRootTable->DescTAG.Get();

	const FbxBindingTable* lTable = pImplementation->GetRootTable();
	size_t lEntryNum = lTable->GetEntryCount();

	for (int i = 0; i < (int)lEntryNum; ++i)
	{
		const FbxBindingTableEntry& lEntry = lTable->GetEntry(i);
		const char* lEntrySrcType = lEntry.GetEntryType(true);
		FbxProperty lFbxProp;


		FbxString lTest = lEntry.GetSource();
		FBXSDK_printf("            Entry: %s\n", lTest.Buffer());


		if (strcmp(FbxPropertyEntryView::sEntryType, lEntrySrcType) == 0)
		{
			lFbxProp = pSurfaceMaterial->FindPropertyHierarchical(lEntry.GetSource());
			if (!lFbxProp.IsValid())
			{
				lFbxProp = pSurfaceMaterial->RootProperty.FindHierarchical(lEntry.GetSource());
			}


		}
		else if (strcmp(FbxConstantEntryView::sEntryType, lEntrySrcType) == 0)
		{
			lFbxProp = pImplementation->GetConstants().FindHierarchical(lEntry.GetSource());
		}
		if (lFbxProp.IsValid())
		{
			if (lFbxProp.GetSrcObjectCount<FbxTexture>() > 0)
			{
				//do what you want with the textures
				for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxFileTexture>(); ++j)
				{
					FbxFileTexture* lTex = lFbxProp.GetSrcObject<FbxFileTexture>(j);
					FBXSDK_printf("           File Texture: %s\n", lTex->GetFileName());
				}
				for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxLayeredTexture>(); ++j)
				{
					FbxLayeredTexture* lTex = lFbxProp.GetSrcObject<FbxLayeredTexture>(j);
					FBXSDK_printf("        Layered Texture: %s\n", lTex->GetName());
				}
				for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxProceduralTexture>(); ++j)
				{
					FbxProceduralTexture* lTex = lFbxProp.GetSrcObject<FbxProceduralTexture>(j);
					FBXSDK_printf("     Procedural Texture: %s\n", lTex->GetName());
				}
			}
			else
			{
				FbxDataType lFbxType = lFbxProp.GetPropertyDataType();
				FbxString blah = lFbxType.GetName();
				if (FbxBoolDT == lFbxType)
				{
				}
				else if (FbxIntDT == lFbxType || FbxEnumDT == lFbxType)
				{
				}
				else if (FbxFloatDT == lFbxType)
				{
				}
				else if (FbxDoubleDT == lFbxType)
				{
				}
				else if (FbxStringDT == lFbxType
					|| FbxUrlDT == lFbxType
					|| FbxXRefUrlDT == lFbxType)
				{
				}
				else if (FbxDouble2DT == lFbxType)
				{
					FbxDouble2 lDouble2 = lFbxProp.Get<FbxDouble2>();
					FbxVector2 lVect;
					lVect[0] = lDouble2[0];
					lVect[1] = lDouble2[1];

				}
				else if (FbxDouble3DT == lFbxType || FbxColor3DT == lFbxType)
				{
					FbxDouble3 lDouble3 = lFbxProp.Get<FbxDouble3>();


					FbxVector4 lVect;
					lVect[0] = lDouble3[0];
					lVect[1] = lDouble3[1];
					lVect[2] = lDouble3[2];
				}
				else if (FbxDouble4DT == lFbxType || FbxColor4DT == lFbxType)
				{
					FbxDouble4 lDouble4 = lFbxProp.Get<FbxDouble4>();
					FbxVector4 lVect;
					lVect[0] = lDouble4[0];
					lVect[1] = lDouble4[1];
					lVect[2] = lDouble4[2];
					lVect[3] = lDouble4[3];
				}
				else if (FbxDouble4x4DT == lFbxType)
				{
					FbxDouble4x4 lDouble44 = lFbxProp.Get<FbxDouble4x4>();
					for (int j = 0; j < 4; ++j)
					{

						FbxVector4 lVect;
						lVect[0] = lDouble44[j][0];
						lVect[1] = lDouble44[j][1];
						lVect[2] = lDouble44[j][2];
						lVect[3] = lDouble44[j][3];
					}
				}
			}

		}
	}
	
}

float FBXMaterialImport::isTransparent(FbxSurfaceMaterial* pSurfaceMaterial)
{
	FbxProperty Prop = pSurfaceMaterial->FindProperty("Opacity");
	FbxProperty Prop2 = pSurfaceMaterial->FindProperty("TransparencyFactor");

	float Value = Prop.Get<FbxFloat>();
	float Value2 = Prop2.Get<FbxFloat>();
	if (Prop.IsValid() && 0 < Value && Value < 1)
	{
		return Value;
	}
	if (Prop2.IsValid() && 0 < Value2 && Value2 < 1)
	{
		return Value2;
	}
	return 1;
}

 const FbxImplementation* FBXMaterialImport::LoadImpl(FbxSurfaceMaterial* pMaterial)
{
	const FbxImplementation* lImplementation = nullptr;
	if (!lImplementation) 
		lImplementation = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_CGFX);
	if (!lImplementation) 
		lImplementation = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_HLSL);
	if (!lImplementation) 
		lImplementation = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_SFX);
	if (!lImplementation) 
		lImplementation = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_OGS);
	if (!lImplementation) 
		lImplementation = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_SSSL);
	return lImplementation;
}

 FLinearColor FBXMaterialImport::ConvertToColor(FbxProperty prop)
 {
	 FLinearColor color;
	 switch (prop.GetPropertyDataType().GetType())
	 {
	 case eFbxDouble3:
	 {
		 FbxDouble3 v = prop.Get<FbxDouble3>();
		 color.R = v.mData[0];
		 color.G = v.mData[1];
		 color.B = v.mData[2];
		 color.A = 0.0f;
		 break;
	 }
	 case eFbxDouble4:
	 {
		 FbxDouble4 v = prop.Get<FbxDouble4>();
		 color.R = v.mData[0];
		 color.G = v.mData[1];
		 color.B = v.mData[2];
		 color.A = v.mData[3];
		 break;
	 }
	 default:
		 color;
	 }
	 return color;
 }

 float FBXMaterialImport::ConvertToFloat(FbxProperty prop)
 {
	 float val = 0.0;
	 switch (prop.GetPropertyDataType().GetType())
	 {
	 case eFbxDouble:
	 {
		 val = prop.Get<FbxDouble>();
		 
		 break;
	 }
	 case eFbxFloat:
	 {
		 val = prop.Get<FbxFloat>();
		 break;
	 }
	 default:
		 val;
	 }
	 return val;
 }
