// Fill out your copyright notice in the Description page of Project Settings.
//TODO try to use planes intead of that cube with low Z value
// Use another material, these are 4K
#include "MC_Chunk.h"
#include "DrawDebugHelpers.h"
// TODO change textures 4k are not working out
AMC_Chunk::AMC_Chunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

AMC_Chunk::~AMC_Chunk()
{

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

void AMC_Chunk::AddVoxel(FTransform Transform, UInstancedStaticMeshComponent* StaticMesh, bool local){
	//Instead of adding one by one and then deleting it, lets calculate all positions of the voxels and then draw only the sides that are seen
	if(StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::AddVoxelInit: StaticMesh is null"));
		return;
	}

	// Check if there is any Voxel next tho this new one(alll 6 sides), and only create sides that are vissible(that doen'st have a neighboor)
	FVector LocalLocation = Transform.GetLocation();
	FVector ChunkLocation = this->GetActorLocation();
	
	//Turn it into World Coordinates unless Transform is local
	FVector NewLocation = LocalLocation;
	if(local){
		NewLocation = NewLocation + ChunkLocation;
	}

	Transform.SetLocation(NewLocation);
	
	StaticMesh->AddInstanceWorldSpace(Transform);
}

void AMC_Chunk::RemoveVoxel(UInstancedStaticMeshComponent* StaticMesh, FHitResult OutHit){

	if(StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: StaticMesh is null"));
		return;
	}

	if(!StaticMesh->RemoveInstance(OutHit.Item)){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: Not Deleting Instance: %d For for %s"), OutHit.Item, *OutHit.GetComponent()->GetName());
		return;
	}
}
