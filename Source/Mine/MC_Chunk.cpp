// Fill out your copyright notice in the Description page of Project Settings.

// Use another material, these are 4K
#include "MC_Chunk.h"
#include "DrawDebugHelpers.h"

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

void AMC_Chunk::AddVoxel(FTransform Transform, bool local, int InstanceIndex){
	// Check if there is any Voxel next tho this new one(alll 6 sides), and only create sides that are vissible(that doen'st have a neighboor)
	FVector LocalLocation = Transform.GetLocation();
	FVector ChunkLocation = GetActorLocation();
	
	//Turn it into World Coordinates unless Transform is local
	FVector NewLocation = LocalLocation;
	if(local){
		NewLocation = NewLocation + ChunkLocation;
	}
	Transform.SetLocation(NewLocation);

	UInstancedStaticMeshComponent* StaticMesh = ISMs[InstanceIndex];

	if(!StaticMesh){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::PutBlock: No Static Mesh"));
		return;
	}
	
	StaticMesh->AddInstanceWorldSpace(Transform);
	if(Voxels.Contains(NewLocation)){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::PutBlock: Vector %s ,  in Voxels map"), *NewLocation.ToString());
		return;
	}
	Voxels.Add(NewLocation, InstanceIndex);
	if(VisibleVoxels.Contains(NewLocation)){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::PutBlock: Vector %s ,  in VisibleVoxels map"), *NewLocation.ToString());
		return;
	}
	VisibleVoxels.Add(NewLocation);

	TArray<FVector> CurrentNeighboors = FindNeighboor(NewLocation);
	for(FVector CurrentNeighboor:CurrentNeighboors){
		TArray<FVector> TempNeighboors = FindNeighboor(CurrentNeighboor);
		if(TempNeighboors.Num() == 6){
			HideVoxelByLocation(CurrentNeighboor);
		}
	}	
}

void AMC_Chunk::RemoveVoxel(UInstancedStaticMeshComponent* StaticMesh, FHitResult OutHit){

	if(StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: StaticMesh is null"));
		return;
	}

	FTransform OutInstanceTransform;
	StaticMesh->GetInstanceTransform(OutHit.Item, OutInstanceTransform, true);
	
	FVector NewLocation = OutInstanceTransform.GetLocation();

	// Show neighboors, since it can be seen now
	TArray<FVector> Neighboor = FindNeighboor(NewLocation);
	if(Neighboor.Num() != 0){
		FTransform Transform;
		int InstanceIndex = ISMs.Find(StaticMesh);
		for(FVector tempVector: Neighboor){
			if(!VisibleVoxels.Contains(tempVector)){
				Transform.SetLocation(tempVector);
				if(!Voxels.Contains(tempVector)){
					UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: Vector %s , is not in Voxels map"), *NewLocation.ToString());
					return;
				}
				// Find StaticMesh of voxel in that position "tempVector"
				UInstancedStaticMeshComponent* NewStaticMesh = ISMs[Voxels[tempVector]];
				if(!NewStaticMesh){
					UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: No Static Mesh"));
					return;
				}
				NewStaticMesh->AddInstanceWorldSpace(Transform);
				VisibleVoxels.Add(tempVector);
			}
		}
	}

	if(!StaticMesh->RemoveInstance(OutHit.Item)){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: Not Deleting Instance: %d For for %s"), OutHit.Item, *OutHit.GetComponent()->GetName());
		return;
	}
	if(!Voxels.Contains(NewLocation)){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: Vector %s , is not in Voxels map"), *NewLocation.ToString());
		return;
	}
	Voxels.Remove(NewLocation);
	VisibleVoxels.Remove(NewLocation);
}

TArray<FVector> AMC_Chunk::FindNeighboor(FVector VoxelLocation){
	TArray<FVector> Result;
	float tempX = VoxelLocation.X;
	float tempY = VoxelLocation.Y;
	float tempZ = VoxelLocation.Z;

	if(Voxels.Contains(FVector(tempX + VoxelSize, tempY, tempZ))){
		Result.Add(FVector(tempX + VoxelSize, tempY, tempZ));
	}
	if(Voxels.Contains(FVector(tempX - VoxelSize, tempY, tempZ))){
		Result.Add(FVector(tempX - VoxelSize, tempY, tempZ));
	}
	if(Voxels.Contains(FVector(tempX, tempY + VoxelSize, tempZ))){
		Result.Add(FVector(tempX, tempY + VoxelSize, tempZ));
	}
	if(Voxels.Contains(FVector(tempX, tempY - VoxelSize, tempZ))){
		Result.Add(FVector(tempX, tempY - VoxelSize, tempZ));
	}
	if(Voxels.Contains(FVector(tempX, tempY, tempZ + VoxelSize))){
		Result.Add(FVector(tempX, tempY, tempZ + VoxelSize));
	}
	if(Voxels.Contains(FVector(tempX, tempY, tempZ - VoxelSize))){
		Result.Add(FVector(tempX, tempY, tempZ - VoxelSize));
	}
	
	return Result;

}

void AMC_Chunk::HideVoxelByLocation(FVector Location){
	FHitResult OutHit;
	FVector EndLocation = FVector(Location.X + VoxelSize,Location.Y, Location.Z);

	bool bSuccess = GetWorld()->LineTraceSingleByChannel(OutHit, EndLocation,Location, ECollisionChannel::ECC_Destructible);
	//DrawDebugLine(GetWorld(), EndLocation, Location,  FColor::Red, true);

	if(bSuccess){
		// Delete from Visible
		VisibleVoxels.Remove(Location);

		// find instance index
		if(!Voxels.Contains(Location)){
			UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::HideVoxelByLocation: Vector %s , is not in Voxels map"), *Location.ToString());
			return;
		}
		int InstanceIndex = Voxels[Location];

		UInstancedStaticMeshComponent* StaticMesh = ISMs[InstanceIndex];
		if(StaticMesh == nullptr){
			UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: StaticMesh is null"));
			return;
		}
		// Remove Instance
		if(!StaticMesh->RemoveInstance(OutHit.Item)){
			UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::HideVoxelByLocation: Not Deleting Instance: %d For for %s"), OutHit.Item, *OutHit.GetComponent()->GetName());
			return;
		}
	} else {
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::HideVoxelByLocation: Didn't hit and should have"));
		return;
	}
}