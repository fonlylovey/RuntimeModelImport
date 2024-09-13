// Fill out your copyright notice in the Description page of Project Settings.
/*
*	ͨ�����Ͷ���
*/
#pragma once
#include "CoreMinimal.h"

UENUM()
enum ERMIImportProgressStage
{
	Stage1_2_ReadMesh,		//
	Stage2_2_CreateSection,	//
};

//

//
const FString RMI_DEFAULTMAT = TEXT("/RuntimeModelImport/Materials/FBXMaterial.FBXMaterial");

//
const FString RMI_TRANSPARENTMAT = TEXT("/RuntimeModelImport/Materials/FBXTransparent.FBXTransparent");

//
#define RMI_BUILTIN_PROGRESUI_PATH TEXT("/RuntimeModelImport/UMG/RMIProgressUMG.RMIProgressUMG_C")

