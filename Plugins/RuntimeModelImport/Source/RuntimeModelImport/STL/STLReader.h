
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
	
    virtual TSharedPtr<FModelMesh> ReadFile(const FString& strPath, const FImportOptions& options) override;

private:
	void ReadAsciiSTL(std::ifstream* File, TSharedPtr<FModelMesh> Model);

	void ReadBinarySTL(std::ifstream* File, TSharedPtr<FModelMesh> Model);

	bool isBinaryFile(std::ifstream* File);

private:
};
