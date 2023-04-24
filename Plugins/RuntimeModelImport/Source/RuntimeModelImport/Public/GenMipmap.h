
#pragma once

#include "CoreMinimal.h"
#include "Memory/SharedBuffer.h"
#include "GenMipmap.generated.h"

USTRUCT()
struct RUNTIMEMODELIMPORT_API FGenMipmap
{
	GENERATED_USTRUCT_BODY();

	void Init(int32 InSizeX, int32 InSizeY, ETextureSourceFormat InFormat, const void* InData, int32 InDataSize);

	void GetMipMap(UTexture2D* InTexture);

	int64 GetMipSize(int32 InMipIndex) const;

	int64 ComputeBufferSize() const;

	int32 NumMips = 0;

	int32 NumSlice = 1;

	int32 SizeX = 0;

	int32 SizeY = 0;

	int32 DataSize = 0;
	TArray<uint8> ByteData;

	ETextureSourceFormat Format = TSF_Invalid;
};
