#include "ModelMaterial.h"
#include "GenMipmap.h"

FModelTexture::~FModelTexture()
{
	BulkData.Empty();
}

FModelMaterial::FModelMaterial(int32 id, FString name)
{
	ID = id;
	Name = name;
	DiffuseFactor = 1.0;
	EmissiveFactor = 1.0;
	Metallic = 0.0;
	Specular = 0.0;
	Roughness = 0.5;
	Opacity = 1.0;
	DepthOffset = 0.0;
	IsTransparent = false;
	DiffuseColor = FLinearColor(1, 1, 1, 1);
	EmissiveColor = FLinearColor(1, 1, 1, 1);
}

FModelMaterial::FModelMaterial()
{
	ID = -999;
	Name = "None";
	DiffuseFactor = 1.0;
	EmissiveFactor = 1.0;
	Metallic = 0.0;
	Specular = 0.0;
	Roughness = 0.5;
	Opacity = 1.0;
	DepthOffset = 0.0;
	IsTransparent = false;
	DiffuseColor = FLinearColor(1, 1, 1, 1);
	EmissiveColor = FLinearColor(1, 1, 1, 1);
}

FModelMaterial::~FModelMaterial()
{
	ID = -999;
	Name = "None";
	DiffuseFactor = 1.0;
	EmissiveFactor = 1.0;
	Metallic = 0.0;
	Specular = 0.0;
	Roughness = 0.5;
	Opacity = 1.0;
	DepthOffset = 0.0;
	IsTransparent = false;
	DiffuseColor = FLinearColor(1, 1, 1, 1);
	EmissiveColor = FLinearColor(1, 1, 1, 1);
}

UTexture2D* FModelTexture::ToTexture(bool genMipmap)
{
	UTexture2D* texture  = UTexture2D::CreateTransient(SizeX, SizeY, PF_B8G8R8A8);
	if (texture != nullptr)
	{
		texture->AddToRoot();
		texture->MipGenSettings = TMGS_Sharpen4;
		void* TextureData = texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(TextureData, BulkData.GetData(), BulkData.Num());
		texture->GetPlatformData()->Mips[0].BulkData.Unlock();
		if(genMipmap)
		{
			FGenMipmap GenMip;
			GenMip.Init(SizeX, SizeY, TSF_BGRA8, BulkData.GetData(), BulkData.Num());
			GenMip.GetMipMap(texture);
		}
		if (IsInGameThread())
		{
			texture->UpdateResource();
		}
		else
		{
			Async(EAsyncExecution::TaskGraphMainThread, [=]()
			{
				texture->UpdateResource();
			});
		}
	}

	return texture;
}
