

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "RuntimeModelMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class RUNTIMEMODELIMPORT_API URuntimeModelMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	URuntimeModelMeshComponent();
	virtual ~URuntimeModelMeshComponent();

	virtual void BeginDestroy() override;

	//
	virtual void CreateMesh(struct FModelMesh* mesh);
	
};
