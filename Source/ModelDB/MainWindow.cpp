// Fill out your copyright notice in the Description page of Project Settings.

#include "MainWindow.h"
#include "CoreMinimal.h"
#include <Components/Button.h>
#include <Components/ProgressBar.h>

#include "ModelReader.h"
#include "RuntimeDBLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Containers/Array.h"

void UMainWindow::NativeConstruct()
{
	m_pProvider = NewObject<UProgressProvider>();
	m_pProvider->SetProgress(Progress_Import, TextBlock_Percent, TextBlock_Info);
}

void UMainWindow::OnClickedImportModel(const FString& strPath)
{
	UModelReader* reader = URuntimeDBLibrary::MakeReader(strPath);
	
	FModelMesh* model = reader->ReadFile();
	for (auto& item : model->Sections)
	{
		item->Vertexes;
	}
}

void UMainWindow::OnClickedSaveModel()
{
	TArray<AActor*> FoundActors; 
	//UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARuntimeActor::StaticClass(), FoundActors);

	for (AActor* pActor : FoundActors)
	{
		//auto runtimeActor = Cast<ARuntimeActor>(pActor);
		//if (runtimeActor->GetIsRoot())
		{
			//Importer::SaveModel(TEXT("E:/Test/"), runtimeActor);
		}
	}

}

void UMainWindow::OnClickedLoadModel()
{
	//Importer::LoadModel(TEXT("E:/Test/"));
}

void UMainWindow::OnFinishImport(AActor* actor)
{

}
