#include "IFCReader.h"

FIFCReader::FIFCReader(const FString& FilePath)
: FReaderBase(FilePath)
{
}

FIFCReader::FIFCReader(const FString& FilePath, const FImportOptions& Options)
: FReaderBase(FilePath, Options)
{
}

TSharedPtr<FModelMesh> FIFCReader::ReadFile(const FString& strPath, const FImportOptions& options)
{
	TSharedPtr<FModelMesh> modelMesh =MakeShared<FModelMesh>(0, "");
	return modelMesh;
}

