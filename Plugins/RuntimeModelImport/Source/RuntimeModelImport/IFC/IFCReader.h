
#pragma once
#include "ReaderBase.h"
#include "ModelMesh.h"

class RUNTIMEMODELIMPORT_API FIFCReader : public FReaderBase
{
public:
	FIFCReader(const FString& FilePath);
	FIFCReader(const FString& FilePath, const FImportOptions& Options);
	
	inline FModelMesh* ReadFile(const FString& strPath) { return ReadFile(strPath, FImportOptions()); };
	FModelMesh* ReadFile(const FString& strPath, const FImportOptions& options);
};
