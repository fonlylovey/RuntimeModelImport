// Fill out your copyright notice in the Description page of Project Settings.
/*
*	ͨ�����Ͷ���
*/
#pragma once
#include "CoreMinimal.h"

UENUM()
enum ERMIImportProgressStage
{
	Stage1_2_ReadMesh,		//��ȡMesh, ռ���ȵ�50%
	Stage2_2_CreateSection,	//����Section, ռ���ȵ�50%
};

//������Ƚ׶�

//Ĭ��Opac����
const FString RMI_DEFAULTMAT = TEXT("/RuntimeModelImport/Materials/FBXMaterial.FBXMaterial");

//Ĭ�ϰ�͸����
const FString RMI_TRANSPARENTMAT = TEXT("/RuntimeModelImport/Materials/FBXTransparent.FBXTransparent");

//������õļ���UI
#define RMI_BUILTIN_PROGRESUI_PATH					TEXT("/RuntimeModelImport/UMG/RMIProgressUMG.RMIProgressUMG_C")

