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
	bool Visible;
	FString Name;
	FVector SideLocation;
	FQuat SideRotation;
	FSide* Next;
	FVoxel* Parent;
	UInstancedStaticMeshComponent* StaticMesh;
	AMC_Chunk* Chunk;

	FSide(){
		Visible = false;
		Name = "";
		SideLocation = FVector(0);
		SideRotation = FQuat(FRotator(0,0,0));
		Next = nullptr;
		Parent = nullptr;
		StaticMesh = nullptr;
		Chunk = nullptr;		
	}

	FString ToString(){
		FString print =  "Name: " + Name + "; " + "SideLocation: " + SideLocation.ToString() + "; " 
		+ "SideRotation: " + SideRotation.ToString() + "; " ;
		if(Next==nullptr){
			print = print + "Next: nullptr; ";
		}
		if(Parent==nullptr){
			print = print + "Parent: nullptr; ";
		}
		if(StaticMesh==nullptr){
			print = print + "StaticMesh: nullptr; ";
		}
		if(Chunk==nullptr){
			print = print + "Chunk: nullptr; ";
		}
		return print;
	}

	~FSide(){
		//
		/*if(Next != nullptr){
			//TODO fix when FSide is destroyed
			FSide TempSide = *Next;
			TempSide.Next = nullptr;
		}*/
	}
};

USTRUCT()
struct FVoxel
{
	GENERATED_BODY()
	FVector VoxelCenter;
	TArray<FSide*> Sides;

	FVoxel(){
		VoxelCenter = FVector(0,0,0);
		Sides.Init(nullptr, 6);
		Sides[0] = nullptr;
		Sides[1] = nullptr;
		Sides[2] = nullptr;
		Sides[3] = nullptr;
		Sides[4] = nullptr;
		Sides[5] = nullptr;		
	}

	~FVoxel(){
		UE_LOG(LogTemp, Warning, TEXT("Deleting Sides"));
		for(int i = 0; i < 6; i++){
			delete Sides[i];
		}
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
	~AMC_Chunk();


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

	TMap<FString, FSide*> Vector2Side;

	TArray<FVoxel*> Voxels;

	void RedrawVoxelSide(FSide* Side);

	FSide* AddVoxelSide(FString Side, FVector Location, FRotator Rotation, UInstancedStaticMeshComponent* StaticMesh, bool AddInstance=true );

	UFUNCTION(BlueprintCallable, Category = "Code")
	void AddVoxel(FTransform Transform, UInstancedStaticMeshComponent* StaticMesh);

	void RemoveVoxelSide(FSide* Side);

	void RemoveVoxel(FVector Location, FQuat Rotation, UInstancedStaticMeshComponent* StaticMesh);
	
	void MapSides(FSide* Side, FHitResult OutHit);

	FString FormatRotator(FRotator Rotator);
};