// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Engine/DataTable.h"
#include "Misc/FileHelper.h"
#include "MC_PlayerController.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FChunkDataTableRow : public FTableRowBase {
	GENERATED_BODY()

	// This will be the Rowname as well
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTable")
	FVector ChunkLocation;
};

USTRUCT(BlueprintType)
struct FVoxelDataTableRow : public FTableRowBase {
	GENERATED_BODY()
	
	// This will be the Rowname
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTable")
	//FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTable")
	int InstanceIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTable")
	FVector ChunkLocation;
};

UCLASS()
class MINE_API AMC_PlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, Category = "CodeInit")
	int ChunkSizeXY = 0;

	TArray<FVector> ChunkCoordinates;
	
public:

	UPROPERTY(BlueprintReadWrite, Category = "CodeInit")
	int seed = 0;

	UPROPERTY(BlueprintReadWrite, Category = "CodeInit")
	int ChunkRange = 10;

	UPROPERTY(BlueprintReadWrite, Category = "CodeInit")
	int ChunkWith = 8; // in Voxels

	UPROPERTY(BlueprintReadWrite, Category = "CodeInit")
	int ChunkDepth = 20; // in Voxels

	UPROPERTY(BlueprintReadWrite, Category = "CodeInit")
	int ChunkVoxelSize = 32;	

	UPROPERTY(BlueprintReadWrite, Category = "Noise")
	float ChunkNoiseDensity = 0.0003;

	UPROPERTY(BlueprintReadWrite, Category = "Noise")
	int ChunkNoiseScale = 7; // in Voxels

	UPROPERTY(BlueprintReadWrite, Category = "Noise")
	float Chunk3dNoiseDensity = 0.0005;

	UPROPERTY(BlueprintReadWrite, Category = "Noise")
	float Chunk3dNoiseCutOffBelowSurface = -0.63;

	UPROPERTY(BlueprintReadWrite, Category = "Noise")
	float Chunk3dNoiseCutOffAtSurface = -0.7;

	FString ChunkDataTableName = TEXT("ChunkDataTable");
	FString VoxelDataTableName = TEXT("VoxelDataTable");
	FString DataDirectory = FPaths::ProjectContentDir() + TEXT("Data/");

	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	UDataTable* ChunkDataTable;
	UDataTable* VoxelDataTable;


	bool CreateChunkDataTableFile(bool New);
	bool CreateVoxelDataTableFile(bool New);

	void GenerateTerrain();

	FVector GetPlayerChunkCoordinates();

};
