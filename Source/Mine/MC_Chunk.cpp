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
	//UE_LOG(LogTemp, Warning, TEXT("AMC_Chunk::~AMC_Chunk: Size Voxels: %d"), Voxels.GetAllocatedSize());
 	/*for(int i = Voxels.Num() - 1; i >= 0 ; i--){
		 delete Voxels[i];
	}*/
	//UE_LOG(LogTemp, Warning, TEXT("AMC_Chunk::~AMC_Chunk: Size Voxels: %d"), Voxels.GetAllocatedSize());
}

// Called when the game starts or when spawned
void AMC_Chunk::BeginPlay()
{
	Super::BeginPlay();
	DrawAllVoxels();
}

// Called every frame
void AMC_Chunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMC_Chunk::DrawVoxelSide(FSide& Side){
	if(Side.StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::DrawVoxelSide: Side.StaticMesh not initialized"));
		return;
	}
	FTransform Transform;
	Transform.SetLocation(Side.SideLocation);
	Transform.SetRotation(Side.SideRotation);
	Side.StaticMesh->AddInstanceWorldSpace(Transform);
	if(!Side.Visible){
		Side.Visible = true;
	}
}

void AMC_Chunk::DrawAllVoxels(){
	for(FSide* VoxelSide : VisibleSides){
		if(VoxelSide->Visible){
			DrawVoxelSide(*VoxelSide);
		} else {
			UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::DrawAllVoxels: There is an InVisible Fside in the VisibleSides list"));
		}
	}
	VisibleSides.Empty();
}

void AMC_Chunk::AddVoxelSide(FSide &NewSide, FString Side, FVector Location, FQuat Rotation_Quat, UInstancedStaticMeshComponent* StaticMesh){
	if(StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::AddVoxelSide: StaticMesh is null"));
		return ;
	}
	FRotator Rotation = Rotation_Quat.Rotator();
	FTransform Transform;
	if(Side == TEXT("Top")){
		Transform.SetLocation(FVector(Location.X, Location.Y, Location.Z + VoxelSize/2));
		Transform.SetRotation(FQuat(Rotation));
	} else if(Side == TEXT("Bottom")){
		Transform.SetLocation(FVector(Location.X, Location.Y, Location.Z - VoxelSize/2));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch + 180, Rotation.Yaw, Rotation.Roll)));
	} else if(Side == TEXT("Front")){
		Transform.SetLocation(FVector(Location.X + VoxelSize/2, Location.Y, Location.Z));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch - 90, Rotation.Yaw,Rotation.Roll)));
	} else if(Side == TEXT("Back")){
		Transform.SetLocation(FVector(Location.X - VoxelSize/2, Location.Y, Location.Z));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch + 90, Rotation.Yaw, Rotation.Roll)));
	} else if(Side == TEXT("Right")){
		Transform.SetLocation(FVector(Location.X, Location.Y - VoxelSize/2, Location.Z));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch, Rotation.Yaw, Rotation.Roll - 90)));
	} else if(Side == TEXT("Left")){
		Transform.SetLocation(FVector(Location.X, Location.Y + VoxelSize/2, Location.Z));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch, Rotation.Yaw, Rotation.Roll + 90)));
	} else {
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::AddVoxelSide: Trying a different side %s"), *Side);
		return;
	}

	NewSide.SideLocation = Transform.GetLocation();
	NewSide.SideRotation = Transform.GetRotation();
	NewSide.StaticMesh = StaticMesh;
	NewSide.Chunk = this;

	NewSide.Visible = true;
	VisibleSides.Add(&NewSide);

	// Checking if there is already 1 Side in same location, whihc means both sides should be visible
	if(Vector2Side.Contains(NewSide.SideLocation)){
		// Map both sides to each other
		FSide* NextSide = Vector2Side[NewSide.SideLocation];
		if(NextSide == nullptr){
			UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::AddVoxelSide: NextSide == nullptr"));
			return;
		}
		if(NextSide->Next != nullptr){
			UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::AddVoxelSide: Vector2Side Already contains: %s with 2 side in that Location"), *Transform.GetLocation().ToString());
			return;
		}
		RemoveVoxelSide(*NextSide);

		NextSide->Next = &NewSide;
		NewSide.Next = NextSide;

		NextSide->Visible = false;
		NewSide.Visible = false;

		VisibleSides.Remove(NextSide);
		VisibleSides.Remove(&NewSide);
		return;
	}
	
	Vector2Side.Add(NewSide.SideLocation, &NewSide);
	// TODO check if there is a hit, this would mean this is the limit of the chunk and there is another block we can map them
	// Find index, we are going to trace a small line crossing through the center of the mesh
	FHitResult OutHit;
	FVector end = NewSide.SideLocation;
	FVector start = FVector(end.X + 1, end.Y + 1, end.Z + 1);

	bool bsuccess = GetWorld()->LineTraceSingleByChannel(OutHit, start, end, ECollisionChannel::ECC_Destructible);
	if(bsuccess){		
		MapSides(NewSide, OutHit);
		VisibleSides.Remove(&NewSide);
	}
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
	FVoxel* NewVoxel = new FVoxel;
	NewVoxel->VoxelCenter = NewLocation;

	// Top
	AddVoxelSide(NewVoxel->Sides[0], TEXT("Top"), NewLocation, Transform.GetRotation(), StaticMesh);
	//Bottom
	AddVoxelSide(NewVoxel->Sides[1], TEXT("Bottom"), NewLocation, Transform.GetRotation(), StaticMesh);	
	//Front
	AddVoxelSide(NewVoxel->Sides[2], TEXT("Front"), NewLocation, Transform.GetRotation(), StaticMesh);
	//Back
	AddVoxelSide(NewVoxel->Sides[3], TEXT("Back"), NewLocation, Transform.GetRotation(), StaticMesh);
	//Right
	AddVoxelSide(NewVoxel->Sides[4], TEXT("Right"), NewLocation, Transform.GetRotation(), StaticMesh);
	//Left
	AddVoxelSide(NewVoxel->Sides[5], TEXT("Left"), NewLocation, Transform.GetRotation(), StaticMesh);

	Voxels.Add(NewVoxel);
}

void AMC_Chunk::RemoveVoxelSide(FSide& Side, FHitResult *InHit){

	if(Side.StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxelSide: StaticMesh is null"));
		return;
	}

	//Removing from Vector2Side
	Vector2Side.Remove(Side.SideLocation);

	//Lets check if curretn side is inVisible and the other side in same location will become visible.
	if(!Side.Visible){
		UE_LOG(LogTemp, Warning, TEXT("AMC_Chunk::RemoveVoxelSide: Removing invisible side"));
		//create instance of the side that is besides to this side(To be deleted) and delete this side
		if(Side.Next == nullptr){
			UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxelSide: Side.Next is null and shouldn't since this side is invicible there have to be a Next"));
			return;
		}
		FSide* NextSide = Side.Next;

		if(NextSide->Chunk == nullptr){
			UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxelSide: NextSide.Chunk is null"));
			return;
		}
		NextSide->Chunk->DrawVoxelSide(*NextSide);
		Vector2Side.Add(Side.SideLocation, NextSide);
		return;
	}

	Side.Visible = false;
	VisibleSides.Remove(&Side);

	if(InHit != nullptr){
		if(!Side.StaticMesh->RemoveInstance(InHit->Item)){
			UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxelSide: Not Deleting Instance: %d"), InHit->Item);
		}
		return;
	}
	//Find index, we are going to trace a small line crossing through the center of the mesh
	FHitResult OutHit;
	FVector end = Side.SideLocation;
	FVector start = FVector(end.X + 1, end.Y + 1, end.Z + 1);

	bool bsuccess = GetWorld()->LineTraceSingleByChannel(OutHit, start, end, ECollisionChannel::ECC_Destructible);
	if(!bsuccess){
		// Visible Side has not be drawn yet
		return;
	}

	UInstancedStaticMeshComponent* HitComponent = Cast<UInstancedStaticMeshComponent>(OutHit.GetComponent());
	AMC_Chunk* HitChunk = Cast<AMC_Chunk>(OutHit.GetActor());
	
	if(!HitComponent){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxelSide: No Component: %s"), *OutHit.ToString());
		return;
	} 
	if (!HitChunk){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxelSide: No Actor"));
		return;
	}

	if(!Side.StaticMesh->RemoveInstance(OutHit.Item)){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxelSide: Not Deleting Instance: %d For for %s"), OutHit.Item, *HitComponent->GetName());
		return;
	}
}

void AMC_Chunk::RemoveVoxel(FVector Location, FQuat Rotation, UInstancedStaticMeshComponent* StaticMesh){

	if(StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: StaticMesh is null"));
		return;
	}

	if(!Vector2Side.Contains(Location)){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: Missing Location: %s"), *Location.ToString());
		return;
	}
	// Use Rotation to find out which of both side in same location is the one to be removed
	FSide* DeleteSide = Vector2Side[Location];
	if(DeleteSide == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: DeleteSide is nullptr for %s"), *Location.ToString());
		return;
	}

	if(DeleteSide->Next != nullptr){
		if(FormatRotator(DeleteSide->SideRotation.Rotator()) != FormatRotator(Rotation.Rotator())){
			DeleteSide = DeleteSide->Next;
			UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: %s VS %s"), *FormatRotator(DeleteSide->SideRotation.Rotator()), *FormatRotator(Rotation.Rotator())); 
		}
	}

	FVoxel* SideParent = DeleteSide->Parent;
	if(SideParent == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: SideParent is nullptr for %s"), *Location.ToString());
		return;
	}

	for(int i = 5; i >= 0; i--){
		RemoveVoxelSide(SideParent->Sides[i]);
	}
	delete SideParent;
}

void AMC_Chunk::MapSides(FSide& Side, FHitResult OutHit){
	// Remove next side from rendering
	UInstancedStaticMeshComponent* HitComponent = Cast<UInstancedStaticMeshComponent>(OutHit.GetComponent());
	AMC_Chunk* HitChunk = Cast<AMC_Chunk>(OutHit.GetActor());
	if(!HitComponent){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::MapSides: No Component: %s"), *OutHit.ToString());
		return;
	}

	if (!HitChunk){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::MapSides: No Actor"));
		return;
	}

	FTransform OutInstanceTransform;
	HitComponent->GetInstanceTransform(OutHit.Item, OutInstanceTransform, true);
	FVector Location = OutInstanceTransform.GetLocation();
	FQuat SideRotation = OutInstanceTransform.GetRotation();

	if(!HitChunk->Vector2Side.Contains(Location)){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::MapSides: Vector2Side doens't contain: %s in chunk %s"), *Location.ToString(), *HitChunk->GetName());
		return;
	}
	
	FSide* NextSide = HitChunk->Vector2Side[Location];
	if(NextSide == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::MapSides: NextSide == nullptr"));
		return;
	}
	// Use Rotation to find which of both sides in same location is the one to be removed
	if(NextSide->Next != nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::MapSides: NextSide has a Next Side and it shouldn't"));
	}
	
	HitChunk->RemoveVoxelSide(*NextSide, &OutHit);
	Side.Visible = false;

	// Map both sides to each other
	NextSide->Next = &Side;
	Side.Next = NextSide;
}

FString AMC_Chunk::FormatRotator(FRotator Rotator){
	float P = Rotator.Pitch;
	float Y = Rotator.Yaw;
	float R = Rotator.Roll;
	return "P=" + FString::SanitizeFloat(round(P)) + " Y=" + FString::SanitizeFloat(round(Y)) +" R=" + FString::SanitizeFloat(round(R));
}