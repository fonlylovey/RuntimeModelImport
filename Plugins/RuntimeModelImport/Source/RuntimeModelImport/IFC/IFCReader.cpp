#include "IFCReader.h"

FIFCReader::FIFCReader(const FString& FilePath)
: FReaderBase(FilePath)
{
}

FIFCReader::FIFCReader(const FString& FilePath, const FImportOptions& Options)
: FReaderBase(FilePath, Options)
{
}

FModelMesh* FIFCReader::ReadFile(const FString& strPath, const FImportOptions& options)
{
	FModelMesh* modelMesh = new FModelMesh(0, "");
	return modelMesh;
}

