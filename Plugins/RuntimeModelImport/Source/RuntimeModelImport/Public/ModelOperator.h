/**
 * 
 */
#pragma once
#include "CoreMinimal.h"
#include "ReaderBase.h"

//需要改成线程安全的类
class FModelOperator
{
public:
	static FModelOperator* Instance();
	
	//根据文件格式创建实际的reader
	TSharedPtr<FReaderBase> CreateReader(const FString& FilePath);

	FQueuedThreadPool& GetThreadPool();
	
private:
	FModelOperator();
	virtual ~FModelOperator();

	//获取文件后缀名，不包含点 （xxxx.png返回png）
	FString GetSuffix(const FString& FilePath);

private:
	static FModelOperator* s_pSelf;

	TMap<int, TSharedPtr<FReaderBase>> ReaderMap;

	TUniquePtr<FQueuedThreadPool> ThreadPool;
};

