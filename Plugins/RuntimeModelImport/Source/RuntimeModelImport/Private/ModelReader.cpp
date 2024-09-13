﻿#include "ModelReader.h"
#include "ModelOperator.h"

UModelReader::UModelReader(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UModelReader::~UModelReader()
{
}

void UModelReader::Init(const FString& FilePath)
{
	ReaderPrivate = FModelOperator::Instance()->CreateReader(FilePath);
	//
	RuntimeDB::FOnLoadStartDelegate::FDelegate dele;
	dele.BindUObject(this, &UModelReader::OnStartEvent);
	OnStart.Add(dele);
	ReaderPrivate->OnStartDelegate = dele;
}

FModelMesh* UModelReader::ReadFile()
{
	return ReaderPrivate->ReadFile();
}

void UModelReader::AsyncBeginLoad()
{
	AsyncPool(FModelOperator::Instance()->GetThreadPool(),
		[=, this]()
		{
			//ReaderPrivate->ReadFile()
		});
}

void UModelReader::AsyncLoading()
{
	
}

void UModelReader::AsyncFinishLoad()
{
	
}

void UModelReader::OnStartEvent(const FString& info)
{
}
