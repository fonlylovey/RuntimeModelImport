// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RMIDelegates.h"
#include "RuntimeActor.h"
#include "ProgressProvider.h"
#include "MainWindow.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class MODELDB_API UMainWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void OnClickedImportModel(const FString& strPath);

	UFUNCTION(BlueprintCallable)
	void OnClickedSaveModel();

	UFUNCTION(BlueprintCallable)
	void OnClickedLoadModel();


	void OnFinishImport(ARuntimeActor* actor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Import;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UProgressBar* Progress_Import;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_Percent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_Info;

	UPROPERTY()
	UProgressProvider* m_pProvider;
};
