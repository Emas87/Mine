// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#include "MC_Chunk.generated.h"


USTRUCT(BlueprintType)
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
		if(Next != nullptr){			
			// remove reference to this Side
			Next->Next = nullptr;			
		}
		
	}
};

USTRUCT(BlueprintType)
struct FVoxel
{
	GENERATED_BODY()
	FVector VoxelCenter;
	UPROPERTY(BlueprintReadWrite, Category = "Code")
	TArray<FSide> Sides;

	FVoxel(){
		VoxelCenter = FVector(0,0,0);
		Sides.Add(FSide());
		Sides.Add(FSide());
		Sides.Add(FSide());
		Sides.Add(FSide());
		Sides.Add(FSide());
		Sides.Add(FSide());

		Sides[0].Name = "Top";
		Sides[0].Parent = this;
		Sides[1].Name = "Bottom";
		Sides[1].Parent = this;
		Sides[2].Name = "Front";
		Sides[2].Parent = this;
		Sides[3].Name = "Back";
		Sides[3].Parent = this;
		Sides[4].Name = "Right";
		Sides[4].Parent = this;
		Sides[5].Name = "Left";
		Sides[5].Parent = this;
	}
	//DELETE pointer and free memory

	~FVoxel(){
		UE_LOG(LogTemp, Warning, TEXT("Deleting Sides"));
		/*for(int i = 0; i < 6; i++){
			delete Sides[i];
		}*/
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

	TMap<FVector, FSide*> Vector2Side;

	TArray<FVoxel*> Voxels;

	TArray<FSide*> VisibleSides;

	void DrawVoxelSide(FSide& Side);

	UFUNCTION(BlueprintCallable, Category = "Code")
	void DrawAllVoxels();

	void AddVoxelSide(FSide &NewSide, FString Side, FVector Location, FQuat Rotation, UInstancedStaticMeshComponent* StaticMesh);

	UFUNCTION(BlueprintCallable, Category = "Code")
	void AddVoxel(FTransform Transform, UInstancedStaticMeshComponent* StaticMesh, bool local=true);

	void RemoveVoxelSide(FSide& Side, FHitResult *InHit = nullptr);

	void RemoveVoxel(FVector Location, FQuat Rotation, UInstancedStaticMeshComponent* StaticMesh);
	
	void MapSides(FSide& Side, FHitResult OutHit);

	FString FormatRotator(FRotator Rotator);
};