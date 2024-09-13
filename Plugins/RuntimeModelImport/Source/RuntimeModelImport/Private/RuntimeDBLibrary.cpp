#include "RuntimeDBLibrary.h"

UModelReader* URuntimeDBLibrary::MakeReader(const FString& FilePath)
{
	UModelReader* render = NewObject<UModelReader>();
	if (render != nullptr)
	{
		render->AddToRoot();
		render->Init(FilePath);
	}
	return render;
}
