// Fill out your copyright notice in the Description page of Project Settings.

#include "MC_Chunk.h"
#include "DrawDebugHelpers.h"

AMC_Chunk::AMC_Chunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
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

void AMC_Chunk::RedrawVoxelSide(FSide* Side){
	if(Side == nullptr){
		UE_LOG(LogTemp, Error, TEXT("StaticMesh is null"));
		return;
	}
	// In case it is already drawn
	if(Side->InstanceIndex != -1){
		return;
	}

	FTransform Transform;
	int32 InstanceNumber;

	Transform.SetLocation(Side->SideCenter);
	Transform.SetRotation(FQuat(Side->SideRotation));
	InstanceNumber = Side->StaticMesh->AddInstance(Transform);

	Side->InstanceIndex = InstanceNumber;
	Instance2Side.Add(FString::FromInt(InstanceNumber) + " " + Side->StaticMesh->GetName() , Side);
}

FSide AMC_Chunk::AddVoxelSide(FString Side, FVector Location, FRotator Rotation, UInstancedStaticMeshComponent* StaticMesh){
	FSide NewSide;
	if(StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("StaticMesh is null"));
		return NewSide;
	}
	FTransform Transform;
	int32 InstanceNumber;
	if(Side == TEXT("Top")){
		Transform.SetLocation(FVector(Location.X, Location.Y, Location.Z + VoxelSize/2));
		Transform.SetRotation(FQuat(Rotation));
		InstanceNumber = StaticMesh->AddInstance(Transform);

	} else if(Side == TEXT("Bottom")){
		Transform.SetLocation(FVector(Location.X, Location.Y, Location.Z - VoxelSize/2));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch + 180, Rotation.Yaw, Rotation.Roll)));
		InstanceNumber = StaticMesh->AddInstance(Transform);

	} else if(Side == TEXT("Front")){
		Transform.SetLocation(FVector(Location.X + VoxelSize/2, Location.Y, Location.Z));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch - 90, Rotation.Yaw,Rotation.Roll)));
		InstanceNumber = StaticMesh->AddInstance(Transform);

	} else if(Side == TEXT("Back")){
		Transform.SetLocation(FVector(Location.X - VoxelSize/2, Location.Y, Location.Z));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch + 90, Rotation.Yaw, Rotation.Roll)));
		InstanceNumber = StaticMesh->AddInstance(Transform);

	} else if(Side == TEXT("Right")){
		Transform.SetLocation(FVector(Location.X, Location.Y + VoxelSize/2, Location.Z));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch, Rotation.Yaw, Rotation.Roll + 90)));
		InstanceNumber = StaticMesh->AddInstance(Transform);

	} else if(Side == TEXT("Left")){
		Transform.SetLocation(FVector(Location.X, Location.Y - VoxelSize/2, Location.Z));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch, Rotation.Yaw, Rotation.Roll - 90)));
		InstanceNumber = StaticMesh->AddInstance(Transform);
	} else {
		UE_LOG(LogTemp, Error, TEXT("Trying a different side %s"), *Side);
		return NewSide;
	}

	if(Instance2Side.Contains(FString::FromInt(InstanceNumber) + " " + StaticMesh->GetName())){
		UE_LOG(LogTemp, Error, TEXT("Instance2Side Already contains: %d for %s in Location %s, and current location is %s"),
		 InstanceNumber, *Side, *(Instance2Side[FString::FromInt(InstanceNumber) + " " + StaticMesh->GetName()])->VoxelCenter.ToString() , *(Location).ToString());
		return NewSide;
	}
	
	NewSide.Name = Side;
	NewSide.InstanceIndex = InstanceNumber;
	NewSide.VoxelCenter = Location;
	NewSide.SideCenter = Transform.GetLocation();
	NewSide.SideRotation = Transform.GetRotation();
	NewSide.StaticMesh = StaticMesh;
	NewSide.Chunk = this;
	Instance2Side.Add(FString::FromInt(InstanceNumber) + " " + StaticMesh->GetName() , &NewSide);
	//return FString::FromInt(InstanceNumber) + " " + StaticMesh->GetName();
	return NewSide;

}

void AMC_Chunk::AddVoxel(FTransform Transform, UInstancedStaticMeshComponent* StaticMesh){

	if(StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("StaticMesh is null"));
		return;
	}
	// Check if there is any Voxel next tho this new one(alll 6 sides), and only create sides that are vissible(that doen'st have a neighboor)
	FVector NewLocation = Transform.GetLocation();
	FHitResult OutHit;
	FVoxel NewVoxel;
	NewVoxel.VoxelCenter = NewLocation;

	// Top
	bool bsuccess = false;
	bsuccess = GetWorld()->LineTraceSingleByChannel(OutHit, NewLocation, FVector(NewLocation.X, NewLocation.Y, NewLocation.Z + VoxelSize), ECollisionChannel::ECC_Destructible);
	if(!bsuccess || !Cast<AMC_Chunk>(OutHit.GetActor())){
		FSide NewSide = AddVoxelSide(TEXT("Top"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh);
		NewVoxel.Sides[0] = & NewSide;
		NewVoxel.Sides[0]->Parent = & NewVoxel;
	} else {		
		DrawDebugPoint(GetWorld(), OutHit.Location, 20, FColor::White, true);
		DrawDebugDirectionalArrow(GetWorld(), NewLocation, FVector(NewLocation.X, NewLocation.Y, NewLocation.Z + VoxelSize), 20, FColor::Red, true);
		MapSides(NewVoxel.Sides[0], OutHit);
	}

	//Bottom
	bsuccess = GetWorld()->LineTraceSingleByChannel(OutHit, NewLocation, FVector(NewLocation.X, NewLocation.Y, NewLocation.Z - VoxelSize), ECollisionChannel::ECC_Destructible);
	if(!bsuccess || !Cast<AMC_Chunk>(OutHit.GetActor())){
		FSide NewSide = AddVoxelSide(TEXT("Bottom"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh);
		NewVoxel.Sides[1] = & NewSide;
		NewVoxel.Sides[1]->Parent = & NewVoxel;
	} else {
		DrawDebugPoint(GetWorld(), OutHit.Location, 40, FColor::White, true);
		DrawDebugDirectionalArrow(GetWorld(), NewLocation, FVector(NewLocation.X, NewLocation.Y, NewLocation.Z - VoxelSize), 20, FColor::Red, true);
		MapSides(NewVoxel.Sides[1], OutHit);	
	}
	
	//Front
	bsuccess = GetWorld()->LineTraceSingleByChannel(OutHit, NewLocation, FVector(NewLocation.X + VoxelSize, NewLocation.Y, NewLocation.Z), ECollisionChannel::ECC_Destructible);
	if(!bsuccess || !Cast<AMC_Chunk>(OutHit.GetActor())){
		FSide NewSide = AddVoxelSide(TEXT("Front"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh);
		NewVoxel.Sides[2] = & NewSide;
		NewVoxel.Sides[2]->Parent = & NewVoxel;
	} else {
		DrawDebugPoint(GetWorld(), OutHit.Location, 40, FColor::White, true);
		DrawDebugDirectionalArrow(GetWorld(), NewLocation, FVector(NewLocation.X + VoxelSize, NewLocation.Y, NewLocation.Z), 20, FColor::Red, true);
		MapSides(NewVoxel.Sides[2], OutHit);
	}

	//Back
	bsuccess = GetWorld()->LineTraceSingleByChannel(OutHit, NewLocation, FVector(NewLocation.X - VoxelSize, NewLocation.Y, NewLocation.Z), ECollisionChannel::ECC_Destructible);
	if(!bsuccess || !Cast<AMC_Chunk>(OutHit.GetActor())){
		FSide NewSide = AddVoxelSide(TEXT("Back"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh);
		NewVoxel.Sides[3] = & NewSide;
		NewVoxel.Sides[3]->Parent = & NewVoxel;
	} else {
		DrawDebugPoint(GetWorld(), OutHit.Location, 40, FColor::White, true);
		DrawDebugDirectionalArrow(GetWorld(), NewLocation, FVector(NewLocation.X - VoxelSize, NewLocation.Y, NewLocation.Z), 20, FColor::Red, true);
		MapSides(NewVoxel.Sides[3], OutHit);
	}

	//Right
	bsuccess = GetWorld()->LineTraceSingleByChannel(OutHit, NewLocation, FVector(NewLocation.X, NewLocation.Y - VoxelSize, NewLocation.Z), ECollisionChannel::ECC_Destructible);
	if(!bsuccess || !Cast<AMC_Chunk>(OutHit.GetActor())){
		FSide NewSide = AddVoxelSide(TEXT("Right"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh);
		NewVoxel.Sides[4] = & NewSide;
		NewVoxel.Sides[4]->Parent = & NewVoxel;
	} else {
		DrawDebugPoint(GetWorld(), OutHit.Location, 40, FColor::White, true);
		DrawDebugDirectionalArrow(GetWorld(), NewLocation, FVector(NewLocation.X, NewLocation.Y - VoxelSize, NewLocation.Z), 20, FColor::Red, true);
		MapSides(NewVoxel.Sides[4], OutHit);
	}

	//Left
	bsuccess = false;
	bsuccess = GetWorld()->LineTraceSingleByChannel(OutHit, NewLocation, FVector(NewLocation.X, NewLocation.Y + VoxelSize, NewLocation.Z), ECollisionChannel::ECC_Destructible);
	if(!bsuccess || !Cast<AMC_Chunk>(OutHit.GetActor())){
		FSide NewSide = AddVoxelSide(TEXT("Left"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh);
		NewVoxel.Sides[5] = & NewSide;
		NewVoxel.Sides[5]->Parent = & NewVoxel;
	} else {
		DrawDebugPoint(GetWorld(), OutHit.Location, 40, FColor::White, true);
		DrawDebugDirectionalArrow(GetWorld(), NewLocation, FVector(NewLocation.X, NewLocation.Y + VoxelSize, NewLocation.Z), 20, FColor::Red, true);
		MapSides(NewVoxel.Sides[5], OutHit);
	}
}

void AMC_Chunk::RemoveVoxelSide(FString IndexComponent, UInstancedStaticMeshComponent* StaticMesh){

	FString Instance;
	FString Component;
	if(!IndexComponent.Split(" ", &Instance, &Component)){
		UE_LOG(LogTemp, Error, TEXT("Couldn't Split %s"), *IndexComponent);
		return;
	}

	if(!StaticMesh->RemoveInstance(FCString::Atoi(*Instance))){
		UE_LOG(LogTemp, Error, TEXT("Not Deleting Instance: %d For for %s"), FCString::Atoi(*Instance), *Component);
		return;
	}
	Instance2Side.Remove(IndexComponent);
	// UE_LOG(LogTemp, Warning, TEXT("Deleting Instance: %d For for %s"), FCString::Atoi(*Instance), *Component)
	FSide* Side = Instance2Side[IndexComponent];
	if(Side == nullptr){
		return;
	}

	Side->InstanceIndex = -1;

	//TODO create instance of the side that is besides to this side(To be deleted) and delete this side
	if(Side->Next == nullptr){
		return;
	}
	FSide* NextSide = Side->Next;
	NextSide->Chunk->RedrawVoxelSide(NextSide);
}

void AMC_Chunk::RemoveVoxel(int32 index, UInstancedStaticMeshComponent* StaticMesh){

	FString IndexComponent = FString::FromInt(index) + " " + StaticMesh->GetName();	
	if(!Instance2Side.Contains(IndexComponent)){
		UE_LOG(LogTemp, Error, TEXT("Missing Instance and Component Combination: %s"), *IndexComponent);
		return;
	}
	FSide* DeleteSide = Instance2Side[IndexComponent];
	FVoxel* SideParent = DeleteSide->Parent;

	for(int i = 5; i >= 0; i--){
		IndexComponent = FString::FromInt(SideParent->Sides[i]->InstanceIndex) + " " + StaticMesh->GetName();	
		RemoveVoxelSide(IndexComponent, StaticMesh);
	}
	delete SideParent;	
}

void AMC_Chunk::MapSides(FSide* Side, FHitResult OutHit){
	if(Side == nullptr){
		return;
	}
	// REmove next side from rendering
	UInstancedStaticMeshComponent* HitComponent = Cast<UInstancedStaticMeshComponent>(OutHit.GetComponent());
	AMC_Chunk* HitChunk = Cast<AMC_Chunk>(OutHit.GetActor());
	if(!HitComponent || !HitChunk){
		UE_LOG(LogTemp, Error, TEXT("No Actor or Component"));
		return;
	}
	FString IndexComponent = FString::FromInt(OutHit.Item) + " " + HitComponent->GetName();
	HitChunk->RemoveVoxelSide(IndexComponent, HitComponent);

	if(!HitChunk->Instance2Side.Contains(IndexComponent)){
		UE_LOG(LogTemp, Error, TEXT("Instance2Side doens't contain: %s"), *IndexComponent);
		return;
	}
	// Map both sides to each other
	FSide* NextSide = HitChunk->Instance2Side[IndexComponent];
	if(NextSide == nullptr){
		return;
	}
	NextSide->Next = Side;
	Side->Next = NextSide;
}

// TODO Cuando borro un instance todos los indices cambian, necesito almacenar los indices y cuando cambien cambien los que estan despues del que borre