

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

	TArray<int32> Indexes;

	TArray<FVector> Normals;

	TArray<FVector2D> TexCoord0;

	FBox BoundBox;
	
	virtual bool Serialize(FArchive& Ar);
};
