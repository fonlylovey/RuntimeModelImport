


#include "RuntimeModelActor.h"


// Sets default values
ARuntimeModelActor::ARuntimeModelActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Model = CreateDefaultSubobject<URuntimeModelMeshComponent>(TEXT("Model"));
	RootComponent = Model;
}

// Called when the game starts or when spawned
void ARuntimeModelActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARuntimeModelActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName propertyName = PropertyChangedEvent.Property->GetFName();
	
	if (propertyName ==
		GET_MEMBER_NAME_CHECKED(ARuntimeModelActor, VectorPath)) 
	{
		
	}       
}

void ARuntimeModelActor::LoadingModel()
{
	
}

// Called every frame
void ARuntimeModelActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

