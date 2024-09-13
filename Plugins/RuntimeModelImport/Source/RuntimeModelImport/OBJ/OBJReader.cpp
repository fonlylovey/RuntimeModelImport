#include "OBJReader.h"

FOBJReader::FOBJReader(const FString& FilePath)
	: FReaderBase(FilePath)
{
	
}

FOBJReader::FOBJReader(const FString& FilePath, const FImportOptions& Options)
	: FReaderBase(FilePath, Options)
{
}

FModelMesh* FOBJReader::ReadFile(const FString& strPath, const FImportOptions& options)
{
	FModelMesh* modelMesh = new FModelMesh(0, "");
	return modelMesh;
}
