
#pragma once
#include "ReaderBase.h"
#include "ModelMesh.h"

//STereoLithography
class RUNTIMEMODELIMPORT_API FSTLReader : public FReaderBase
{
public:
	FSTLReader(const FString& FilePath);
	FSTLReader(const FString& FilePath, const FImportOptions& Options);
	virtual ~FSTLReader();
	
	virtual FModelMesh* ReadFile(const FString& strPath, const FImportOptions& options) override;

private:
	void ReadAsciiSTL(std::ifstream* File, FModelMesh* Model);

	void ReadBinarySTL(std::ifstream* File, FModelMesh* &Model);

	bool isBinaryFile(std::ifstream* File);

private:
};
