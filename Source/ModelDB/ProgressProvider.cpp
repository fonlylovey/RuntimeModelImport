// Fill out your copyright notice in the Description page of Project Settings.


#include "ProgressProvider.h"
#include "MainWindow.h"
#include <Components/Button.h>


UProgressProvider::UProgressProvider()
{
	FRMIDelegates::OnImportProgressDelegate.AddUObject(this, &UProgressProvider::UpdateProgress);
}

void UProgressProvider::SetProgress(UProgressBar* progress, UTextBlock* percent, UTextBlock* info)
{
	m_pProgress = progress;
	m_pPercent = percent;
	m_pTextInfo = info;
}

void UProgressProvider::UpdateProgress(int step, int32 index, int32 count, const FString& info)
{
	if (m_pProgress != nullptr)
	{
		float percent = index / (float)count;
		m_pProgress->SetPercent(percent);
		int num = percent * 100;
		FString strNum = FString::SanitizeFloat(num, 0);
		if (m_pPercent != nullptr)
		{
			m_pPercent->SetText(FText::FromString(strNum + "%"));
		}
		
		if (m_pTextInfo != nullptr)
		{
			m_pTextInfo->SetText(FText::FromString(info));
		}
	}
}
