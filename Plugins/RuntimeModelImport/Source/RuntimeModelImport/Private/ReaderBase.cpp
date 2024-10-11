#include "ReaderBase.h"

FImportOptions::FImportOptions()
{

}

FReaderBase::FReaderBase(const FString& FilePath)
: ModelFilePath(FilePath)
{
}

FReaderBase::FReaderBase(const FString& FilePath, const FImportOptions& Options)
: ModelFilePath(FilePath),
  ImportOptions(Options)
{
}

FReaderBase::~FReaderBase()
{
}

TSharedPtr<FModelMesh> FReaderBase::ReadFile()
{
  return ReadFile(ModelFilePath);
}

TSharedPtr<FModelMesh> FReaderBase::ReadFile(const FString& FilePath)
{
  ModelFilePath = FilePath;
  FImportOptions Options;
  return ReadFile(ModelFilePath, Options);
}

