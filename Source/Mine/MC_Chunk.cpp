// Fill out your copyright notice in the Description page of Project Settings.


#include "MC_Chunk.h"


AMC_Chunk::AMC_Chunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMC_Chunk::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMC_Chunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMC_Chunk::AddArray2Coord(FVector Coordinates, FInstancesArray Instances){
	Coord2Array.Add(Coordinates, Instances);
}

int32 AMC_Chunk::AddVoxelSide(FString Side, FVector Location, FRotator Rotation, UInstancedStaticMeshComponent* StaticMesh){

	FTransform Transform;
	int32 InstanceNumber;
	if(Side == TEXT("Top")){
		Transform.SetLocation(FVector(Location.X, Location.Y, Location.Z + VoxelSize/2));
		Transform.SetRotation(FQuat(Rotation));
		InstanceNumber = StaticMesh->AddInstance(Transform);

	} else if(Side == TEXT("Bottom")){
		Transform.SetLocation(FVector(Location.X, Location.Y, Location.Z - VoxelSize/2));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch + 180, Rotation.Yaw, Rotation.Roll)));
		InstanceNumber = StaticMesh->AddInstance(Transform);

	} else if(Side == TEXT("Front")){
		Transform.SetLocation(FVector(Location.X + VoxelSize/2, Location.Y, Location.Z));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch - 90, Rotation.Yaw,Rotation.Roll)));
		InstanceNumber = StaticMesh->AddInstance(Transform);

	} else if(Side == TEXT("Back")){
		Transform.SetLocation(FVector(Location.X - VoxelSize/2, Location.Y, Location.Z));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch + 90, Rotation.Yaw, Rotation.Roll)));
		InstanceNumber = StaticMesh->AddInstance(Transform);

	} else if(Side == TEXT("Right")){
		Transform.SetLocation(FVector(Location.X, Location.Y - VoxelSize/2, Location.Z));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch, Rotation.Yaw, Rotation.Roll - 90)));
		InstanceNumber = StaticMesh->AddInstance(Transform);

	} else if(Side == TEXT("Left")){
		Transform.SetLocation(FVector(Location.X, Location.Y + VoxelSize/2, Location.Z));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch, Rotation.Yaw, Rotation.Roll + 90)));
		InstanceNumber = StaticMesh->AddInstance(Transform);
	} else {
		return -1;
	}
	UE_LOG(LogTemp, Warning, TEXT("Coordinnates of %s plane: %s"), *Side,*Transform.GetLocation().ToString())

	Instance2Coord.Add(InstanceNumber, Location);
	return InstanceNumber;

}

