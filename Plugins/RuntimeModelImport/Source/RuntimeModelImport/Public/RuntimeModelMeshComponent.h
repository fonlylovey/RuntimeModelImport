

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "RuntimeModelMeshComponent.generated.h"

/**
 * 
 */

struct FModelMesh;

UCLASS()
class RUNTIMEMODELIMPORT_API URuntimeModelMeshComponent : public UProceduralMeshComponent
{
	GENERATED_BODY()

public:
	URuntimeModelMeshComponent(const FObjectInitializer& ObjectInitializer);
	virtual ~URuntimeModelMeshComponent();

	virtual void BeginDestroy() override;

	//
	virtual void CreateMesh(TSharedPtr<FModelMesh> mesh);
	
};
