// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#include "MC_Chunk.generated.h"

UCLASS()
class MINE_API AMC_Chunk : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMC_Chunk();
	~AMC_Chunk();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, Category = "Code")
	TArray<UInstancedStaticMeshComponent*> ISMs;

	UPROPERTY(BlueprintReadWrite, Category = "Code")
	TMap<FVector,int> Voxels;
	
	UPROPERTY(BlueprintReadWrite, Category = "Code")
	TMap<FVector,int> NewVoxels;

	UPROPERTY(BlueprintReadWrite, Category = "Code")
	TSet<FVector> VisibleVoxels;

	UPROPERTY(BlueprintReadWrite, Category = "Code", Meta = (ExposeOnSpawn=true, InstanceEditable=true))
	float VoxelSize = 32;

	UFUNCTION(BlueprintCallable, Category = "Code")
	void AddVoxel(FTransform Transform, bool local=true, int InstanceIndex=0);

	void RemoveVoxel(UInstancedStaticMeshComponent* StaticMesh, FHitResult OutHit);

	UFUNCTION(BlueprintCallable, Category = "Code")
	TArray<FVector> FindNeighboor(FVector VoxelLocation);

	void HideVoxelByLocation(FVector Location);
};