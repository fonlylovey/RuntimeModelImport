#include "GenMipmap.h"

void FGenMipmap::Init(int32 InSizeX, int32 InSizeY, ETextureSourceFormat InFormat, const void* InData, int32 InDataSize)
{
	SizeX = InSizeX;
	SizeY = InSizeY;
	Format = InFormat;
	int32 side = SizeX > SizeY ? SizeX : SizeY;
	int numLevels = FMath::Log2(side);
	NumMips = numLevels + 1;
	if(InData)
	{
		int32 size = ComputeBufferSize();
		ByteData.Reset(size);
		ByteData.Append((uint8*)InData, InDataSize);
	}
}

void FGenMipmap::GetMipMap(UTexture2D* InTexture)
{
	const int64 BytesPerPixel = FTextureSource::GetBytesPerPixel(Format);

	TArray<uint8>  mipData;
	auto* priorData = ByteData.GetData();
	int priorwidth = SizeX;
	int priorheight = SizeY;
	while(NumMips > 0)
	{
		int32 MipSizeX = priorwidth >> 1;
		int32 MipSizeY =priorheight >> 1;

		if((MipSizeX == 0) || (MipSizeY == 0) )
		{
			break;;
		}
		if ((MipSizeX > 0) && (MipSizeY > 0))
		{
			mipData.Reset();
			mipData.AddUninitialized(MipSizeX * MipSizeY * 4);

			int dataPerRow = priorwidth * 4;

			//Average out the values
			auto* dataOut = mipData.GetData();
			for (int y = 0; y < MipSizeY; y++)
			{
				auto* dataInRow0 = priorData + (dataPerRow * y * 2);
				auto* dataInRow1 = dataInRow0 + dataPerRow;
				for (int x = 0; x < MipSizeX; x++)
				{
					int totalB = *dataInRow0++;
					int totalG = *dataInRow0++;
					int totalR = *dataInRow0++;
					int totalA = *dataInRow0++;
					totalB += *dataInRow0++;
					totalG += *dataInRow0++;
					totalR += *dataInRow0++;
					totalA += *dataInRow0++;

					totalB += *dataInRow1++;
					totalG += *dataInRow1++;
					totalR += *dataInRow1++;
					totalA += *dataInRow1++;
					totalB += *dataInRow1++;
					totalG += *dataInRow1++;
					totalR += *dataInRow1++;
					totalA += *dataInRow1++;

					totalB >>= 2;
					totalG >>= 2;
					totalR >>= 2;
					totalA >>= 2;

					*dataOut++ = (uint8)totalB;
					*dataOut++ = (uint8)totalG;
					*dataOut++ = (uint8)totalR;
					*dataOut++ = (uint8)totalA;
				}
				dataInRow0 += priorwidth * 2;
				dataInRow1 += priorwidth * 2;
			}

			// Allocate next mipmap.
			FTexture2DMipMap* Mip = new FTexture2DMipMap();
			Mip->SizeX = MipSizeX;
			Mip->SizeY = MipSizeY;
			Mip->BulkData.Lock(LOCK_READ_WRITE);
			void* mipDataPtr = Mip->BulkData.Realloc(mipData.Num());
			FMemory::Memcpy(mipDataPtr, mipData.GetData(), mipData.Num());
			Mip->BulkData.Unlock();
			InTexture->GetPlatformData()->Mips.Add(Mip);
			priorData = mipData.GetData();
			priorwidth = MipSizeX;
			priorheight = MipSizeY;
			
		}
	}
}

int64 FGenMipmap::GetMipSize(int32 InMipIndex) const
{
	const int64 BytesPerPixel = FTextureSource::GetBytesPerPixel(Format);

	int32 MipSizeX = FMath::Max<int32>(SizeX >> InMipIndex, 1);
	int32 MipSizeY = FMath::Max<int32>(SizeY >> InMipIndex, 1);
	int32 MipSizeZ = 1;

	const int64 MipSize = MipSizeX * MipSizeY * MipSizeZ * BytesPerPixel;

	return MipSize;
}

int64 FGenMipmap::ComputeBufferSize() const
{
	int64 TotalSize = 0;
	for (int32 MipIndex = 0; MipIndex < NumMips; ++MipIndex)
	{
		TotalSize += GetMipSize(MipIndex);
	}

	return TotalSize;
}
