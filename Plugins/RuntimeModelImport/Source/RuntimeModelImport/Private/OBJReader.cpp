#include "OBJReader.h"

FModelMesh* OBJReader::ReadFile(const FString& strPath, const FImportOptions& options)
{
	FModelMesh* modelMesh = new FModelMesh(0, "");
	return modelMesh;
}
