// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#include "MC_Chunk.generated.h"


USTRUCT()
struct FSide
{
	GENERATED_BODY()
	int32 InstanceIndex = -1;
	FString Name;
	FVector VoxelCenter;
	FVector SideCenter;
	FQuat SideRotation;
	FVoxel* Parent;
	FSide* Next;
	UInstancedStaticMeshComponent* StaticMesh;
	AMC_Chunk* Chunk;

	~FSide(){
		if(Next != nullptr){
			Next->Next = nullptr;
		}	
	}
};

USTRUCT()
struct FVoxel
{
	GENERATED_BODY()
	FVector VoxelCenter;
	TArray<FSide*> Sides;
	//Sides.Init(nullptr, 6);
	/*FSide* Top;
	FSide* Bottom;
	FSide* Front;
	FSide* Back;
	FSide* Right;
	FSide* Left;*/

	FVoxel(){
		Sides.Init(nullptr, 6);
	}

	~FVoxel(){
		Sides.Empty();
	}

};

UCLASS()
class MINE_API AMC_Chunk : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMC_Chunk();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, Category = "Code")
	TArray<UInstancedStaticMeshComponent*> ISMs;

	UPROPERTY(BlueprintReadWrite, Category = "Code", Meta = (ExposeOnSpawn=true, InstanceEditable=true))
	float VoxelSize = 100;

	TMap<FString, FSide*> Instance2Side;

	FSide AddVoxelSide(FString Side, FVector Location, FRotator Rotation, UInstancedStaticMeshComponent* StaticMesh);

	void AddVoxel(FTransform Transform, UInstancedStaticMeshComponent* StaticMesh);

	void RemoveVoxel(int32 index, UInstancedStaticMeshComponent* StaticMesh);

	void RemoveVoxelSide(FString InstanceComponent, UInstancedStaticMeshComponent* StaticMesh);

	void RedrawVoxelSide(FSide* Side);

	void MapSides(FSide* Side, FHitResult OutHit);
};