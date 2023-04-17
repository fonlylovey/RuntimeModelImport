
#pragma once
#include "ReaderInterface.h"
#include "ModelMesh.h"

class RUNTIMEMODELIMPORT_API OBJReader : public IReaderInterface
{
public:
	inline FModelMesh* ReadFile(const FString& strPath) { return ReadFile(strPath, FImportOptions()); };
	FModelMesh* ReadFile(const FString& strPath, const FImportOptions& options);
};

