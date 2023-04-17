# ModelDB

模型导入插件

## 使用说明

### 1、在项目的XXX.Build.cs中添加依赖名称

### 2、外部主要使用文件 #include "../Importer.h" 和 #include "RMIDelegates.h"两个文件

### RMIDelegates.h 定了了部分委托，主要功能是广播更新模型导入的进度信息，模型导入完成的广播Actor
```
FRMIDelegates::OnImportProgressDelegate.AddUObject(this, &UProgressProvider::UpdateProgress);


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
```

```
    FRMIDelegates::OnImportCompleteDelegate.AddUObject(this, &UMainWindow::OnFinishImport);


    void UMainWindow::OnFinishImport(ARuntimeActor* actor)
    {

    }
```

### Importer.h 包含三个导出函数：

#### ARuntimeActor* LoadModel(const FString& strPath, const FImportOptions& options); strPath表示FBX模型文件的绝对路径，options是预留的其他参数的结构体，当有需要新增加参数时改变这个结构体即可，目前只使用了MergeByMaterial按材质和别这一个bool值

#### 模型导入后的坐标直接修改ARuntimeActor* 返回值，或者在OnImportCompleteDelegate回调中设置也可以，保存模型和再次读取模型时，会将绝对坐标系下的Transform同步保存

```
    FImportOptions option;
    option.MergeByMaterial = false;
    Importer::LoadModel(strPath, option);
```

#### void SaveModel(const FString& saveDir, ARuntimeActor* actor); saveDir 保存模型文件的目录，需要保存的actor, 模型会被保存为以UUID_x.mesh，x为数字编号的文件到本地，大于2G的文件会分为多个文件保存

```
    void UMainWindow::OnClickedSaveModel()
    {
        TArray<AActor*> FoundActors; 
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARuntimeActor::StaticClass(), FoundActors);

        for (AActor* pActor : FoundActors)
        {
            auto runtimeActor = Cast<ARuntimeActor>(pActor);
            if (runtimeActor->GetIsRoot())
            {
                Importer::SaveModel(TEXT("E:/Test/"), runtimeActor);
            }
        }

    }
```
#### TArray<ARuntimeActor*> LoadModel(const FString& saveDir);saveDir 保存模型文件的目录，读取目录下的所有mesh文件，自动在关卡中创建Actor

```
    Importer::LoadModel(TEXT("E:/Test/"));

```