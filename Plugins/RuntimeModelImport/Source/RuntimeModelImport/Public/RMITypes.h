// Fill out your copyright notice in the Description page of Project Settings.
/*
*	通用类型定义
*/
#pragma once
#include "CoreMinimal.h"

UENUM()
enum ERMIImportProgressStage
{
	Stage1_2_ReadMesh,		//读取Mesh, 占进度的50%
	Stage2_2_CreateSection,	//创建Section, 占进度的50%
};

//导入进度阶段

//默认Opac材质
const FString RMI_DEFAULTMAT = TEXT("/RuntimeModelImport/Materials/FBXMaterial.FBXMaterial");

//默认半透材质
const FString RMI_TRANSPARENTMAT = TEXT("/RuntimeModelImport/Materials/FBXTransparent.FBXTransparent");

//插件内置的加载UI
#define RMI_BUILTIN_PROGRESUI_PATH					TEXT("/RuntimeModelImport/UMG/RMIProgressUMG.RMIProgressUMG_C")

