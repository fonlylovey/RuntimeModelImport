
#pragma once
#include "ReaderBase.h"
#include "ModelMesh.h"

class RUNTIMEMODELIMPORT_API FOBJReader : public FReaderBase
{
public:
	FOBJReader(const FString& FilePath);

	FOBJReader(const FString& FilePath, const FImportOptions& Options);
	
	inline FModelMesh* ReadFile(const FString& strPath) { return ReadFile(strPath, FImportOptions()); };
	FModelMesh* ReadFile(const FString& strPath, const FImportOptions& options);
};

