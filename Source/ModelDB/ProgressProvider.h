// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RMIDelegates.h"
#include <Components/ProgressBar.h>
#include "Components/TextBlock.h"
#include "ProgressProvider.generated.h"

/**
 * 
 */
UCLASS()
class MODELDB_API UProgressProvider : public UObject
{
	GENERATED_BODY()

public:
	UProgressProvider();

	void SetProgress(UProgressBar* progress, UTextBlock* percent, UTextBlock* info);

	void UpdateProgress(int step, int32 index, int32 count, const FString& info);

private:
	UProgressBar* m_pProgress;
	UTextBlock* m_pPercent;
	UTextBlock* m_pTextInfo;
};
