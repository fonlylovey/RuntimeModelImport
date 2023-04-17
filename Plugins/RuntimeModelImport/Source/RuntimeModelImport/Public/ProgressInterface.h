/**
 * reader 各种模型的接口类
 */

#pragma once
#include "CoreMinimal.h"

class IProgressInterface
{
public:
	virtual void SetProgressRange(int minValue, int maxValue) = 0;
	virtual void UpdateProgress(float value, FString& info) = 0;
	virtual void ShowProgress() = 0;
	virtual void HideProgress() = 0;
};
