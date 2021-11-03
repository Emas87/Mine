// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#include "MC_Chunk.generated.h"

 USTRUCT(BlueprintType)
 struct FInstancesArray
 {
	 GENERATED_BODY()
     UPROPERTY(BlueprintReadWrite)
     TArray<int32> array;
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

	UPROPERTY(BlueprintReadWrite, Category = "Code")
	TMap<int32, FVector> Instance2Coord;

	UPROPERTY(BlueprintReadWrite, Category = "Code")
	TMap<FVector, FInstancesArray> Coord2Array;

	UFUNCTION(BlueprintCallable, Category = "Code")
	void AddArray2Coord(FVector Coordinates, FInstancesArray Instances);

	//TODO define AddTopInstance, AddBottomInstance, AddFrontInstance, AddBackInstance, AddRightInstance, AddLeftInstance
	UFUNCTION(BlueprintCallable, Category = "Code")
	int32 AddVoxelSide(FString Side, FVector Location, FRotator Rotation, UInstancedStaticMeshComponent* StaticMesh);


	

};

