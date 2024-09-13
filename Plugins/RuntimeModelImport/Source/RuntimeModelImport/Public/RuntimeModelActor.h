#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RuntimeModelMeshComponent.h"
#include "RuntimeModelActor.generated.h"

UCLASS()
class RUNTIMEMODELIMPORT_API ARuntimeModelActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARuntimeModelActor();

protected:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void LoadingModel();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VectorPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	URuntimeModelMeshComponent* Model;

	FString strDefaultMat;
	FString strTransparentMat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* DefaultMat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* TransparentMat;
};
