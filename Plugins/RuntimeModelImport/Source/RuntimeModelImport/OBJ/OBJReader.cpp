#include "OBJReader.h"

FOBJReader::FOBJReader(const FString& FilePath)
	: FReaderBase(FilePath)
{
	
}

FOBJReader::FOBJReader(const FString& FilePath, const FImportOptions& Options)
	: FReaderBase(FilePath, Options)
{
}

TSharedPtr<FModelMesh> FOBJReader::ReadFile(const FString& strPath, const FImportOptions& options)
{
	TSharedPtr<FModelMesh> modelMesh = MakeShared<FModelMesh>(0, "");
	return modelMesh;
}
