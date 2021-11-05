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
	FString Name;
	FVector VoxelCenter;
	FVector SideCenter;
	FQuat SideRotation;
	FSide* Next;
	FVoxel* Parent;
	UInstancedStaticMeshComponent* StaticMesh;
	AMC_Chunk* Chunk;
	TDoubleLinkedListNode<int32>* InstanceIndex;

	FSide(){
		InstanceIndex = nullptr;
		Name = "";
		VoxelCenter = FVector(0);
		SideCenter = FVector(0);
		SideRotation = FQuat(FRotator(0,0,0));
		Next = nullptr;
		Parent = nullptr;
		StaticMesh = nullptr;
		Chunk = nullptr;		
	}

	FString ToString(){
		FString print =  "Name: " + Name + "; " + "VoxelCenter: " + VoxelCenter.ToString() + "; " + "SideCenter: " + SideCenter.ToString() + "; " 
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
		Sides.Init(nullptr, 6);
		VoxelCenter = FVector(0,0,0);
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

	TMap<FString, FSide*> Instance2Side;

	TArray<FVoxel*> Voxels;

	TMap<UInstancedStaticMeshComponent,TDoubleLinkedList<int32>> Indexes;

	FSide* AddVoxelSide(FString Side, FVector Location, FRotator Rotation, UInstancedStaticMeshComponent* StaticMesh, bool AddInstance=true );

	UFUNCTION(BlueprintCallable, Category = "Code")
	void AddVoxel(FTransform Transform, UInstancedStaticMeshComponent* StaticMesh);

	void RemoveVoxel(int32 index, UInstancedStaticMeshComponent* StaticMesh);

	void RemoveVoxelSide(FString InstanceComponent, UInstancedStaticMeshComponent* StaticMesh);

	void RedrawVoxelSide(FSide* Side);

	void MapSides(FSide* Side, FHitResult OutHit);
};