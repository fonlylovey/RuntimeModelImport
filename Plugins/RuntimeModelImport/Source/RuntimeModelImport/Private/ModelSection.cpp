#include "ModelSection.h"

FModelSection::FModelSection()
{
}

FModelSection::~FModelSection()
{
}

bool FModelSection::Serialize(FArchive& Ar)
{
	Ar << SectionIndex;
	Ar << Vertexes;
	return false;
}
