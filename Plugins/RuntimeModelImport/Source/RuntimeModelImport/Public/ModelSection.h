

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class RUNTIMEMODELIMPORT_API FModelSection
{
public:
	FModelSection();
	virtual ~FModelSection();

	int64 SectionIndex = 0;

	TArray<FVector> Vertexes;

	TArray<uint32> Indexes;

	TArray<FVector3f> Normals;

	TArray<FVector2f> TexCoord0;

	FBox BoundBox;
	
	virtual bool Serialize(FArchive& Ar);
};
