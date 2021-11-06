// Fill out your copyright notice in the Description page of Project Settings.

#include "MC_Chunk.h"
#include "DrawDebugHelpers.h"

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
}

// Called every frame
void AMC_Chunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMC_Chunk::RedrawVoxelSide(FSide* Side){
	if(Side == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RedrawVoxelSide: StaticMesh is null"));
		return;
	}

	if(Side->StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RedrawVoxelSide: Side->StaticMesh not initialized"));
		return;
	}

	FTransform Transform;
	Transform.SetLocation(Side->SideLocation);
	Transform.SetRotation(FQuat(Side->SideRotation));
	Side->StaticMesh->AddInstance(Transform);
	Side->Visible = true;
}

FSide* AMC_Chunk::AddVoxelSide(FString Side, FVector Location, FRotator Rotation, UInstancedStaticMeshComponent* StaticMesh, bool AddInstance){
	FSide* NewSide = new FSide;
	if(StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::AddVoxelSide: StaticMesh is null"));
		return NewSide;
	}
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
		Transform.SetLocation(FVector(Location.X, Location.Y + VoxelSize/2, Location.Z));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch, Rotation.Yaw, Rotation.Roll + 90)));
	} else if(Side == TEXT("Left")){
		Transform.SetLocation(FVector(Location.X, Location.Y - VoxelSize/2, Location.Z));
		Transform.SetRotation(FQuat(FRotator(Rotation.Pitch, Rotation.Yaw, Rotation.Roll - 90)));
	} else {
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::AddVoxelSide: Trying a different side %s"), *Side);
		return NewSide;
	}

	if(AddInstance){
		StaticMesh->AddInstance(Transform);
	}

	if(Vector2Side.Contains(Transform.GetLocation())){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::AddVoxelSide: Vector2Side Already contains: %s "), *Transform.GetLocation().ToString());
		return NewSide;
	}

	NewSide->Name = Side;
	NewSide->Visible = true;
	NewSide->SideLocation = Transform.GetLocation();
	NewSide->SideRotation = Transform.GetRotation();
	NewSide->StaticMesh = StaticMesh;
	NewSide->Chunk = this;
	Vector2Side.Add( Transform.GetLocation() , NewSide);
	return NewSide;
}

void AMC_Chunk::AddVoxel(FTransform Transform, UInstancedStaticMeshComponent* StaticMesh){

	if(StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::AddVoxel: StaticMesh is null"));
		return;
	}
	// Check if there is any Voxel next tho this new one(alll 6 sides), and only create sides that are vissible(that doen'st have a neighboor)
	FVector NewLocation = Transform.GetLocation();
	FHitResult OutHit;
	FVoxel* NewVoxel = new FVoxel;
	NewVoxel->VoxelCenter = NewLocation;

	// Top
	bool bsuccess = false;
	bsuccess = GetWorld()->LineTraceSingleByChannel(OutHit, NewLocation, FVector(NewLocation.X, NewLocation.Y, NewLocation.Z + VoxelSize), ECollisionChannel::ECC_Destructible);
	if(!bsuccess || !Cast<AMC_Chunk>(OutHit.GetActor())){
		NewVoxel->Sides[0] = AddVoxelSide(TEXT("Top"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh);
		NewVoxel->Sides[0]->Parent = NewVoxel;
	} else {	
		//AddVoxel without Instance the mesh	
		DrawDebugPoint(GetWorld(), OutHit.Location, 20, FColor::White, true);
		DrawDebugDirectionalArrow(GetWorld(), NewLocation, FVector(NewLocation.X, NewLocation.Y, NewLocation.Z + VoxelSize), 20, FColor::Red, true);
		NewVoxel->Sides[0] = AddVoxelSide(TEXT("Top"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh, false);
		NewVoxel->Sides[0]->Parent = NewVoxel;
		MapSides(NewVoxel->Sides[0], OutHit);
	}
	
	//Bottom
	bsuccess = GetWorld()->LineTraceSingleByChannel(OutHit, NewLocation, FVector(NewLocation.X, NewLocation.Y, NewLocation.Z - VoxelSize), ECollisionChannel::ECC_Destructible);
	if(!bsuccess || !Cast<AMC_Chunk>(OutHit.GetActor())){
		NewVoxel->Sides[1] = AddVoxelSide(TEXT("Bottom"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh);
		NewVoxel->Sides[1]->Parent = NewVoxel;
	} else {
		DrawDebugPoint(GetWorld(), OutHit.Location, 40, FColor::White, true);
		DrawDebugDirectionalArrow(GetWorld(), NewLocation, FVector(NewLocation.X, NewLocation.Y, NewLocation.Z - VoxelSize), 20, FColor::Red, true);
		NewVoxel->Sides[1] = AddVoxelSide(TEXT("Bottom"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh, false);
		NewVoxel->Sides[1]->Parent = NewVoxel;
		MapSides(NewVoxel->Sides[1], OutHit);	
	}
	
	//Front
	bsuccess = GetWorld()->LineTraceSingleByChannel(OutHit, NewLocation, FVector(NewLocation.X + VoxelSize, NewLocation.Y, NewLocation.Z), ECollisionChannel::ECC_Destructible);
	if(!bsuccess || !Cast<AMC_Chunk>(OutHit.GetActor())){
		NewVoxel->Sides[2] = AddVoxelSide(TEXT("Front"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh);
		NewVoxel->Sides[2]->Parent = NewVoxel;
	} else {
		DrawDebugPoint(GetWorld(), OutHit.Location, 40, FColor::White, true);
		DrawDebugDirectionalArrow(GetWorld(), NewLocation, FVector(NewLocation.X + VoxelSize, NewLocation.Y, NewLocation.Z), 20, FColor::Red, true);
		NewVoxel->Sides[2] = AddVoxelSide(TEXT("Front"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh, false);
		NewVoxel->Sides[2]->Parent = NewVoxel;
		MapSides(NewVoxel->Sides[2], OutHit);
	}

	//Back
	bsuccess = GetWorld()->LineTraceSingleByChannel(OutHit, NewLocation, FVector(NewLocation.X - VoxelSize, NewLocation.Y, NewLocation.Z), ECollisionChannel::ECC_Destructible);
	if(!bsuccess || !Cast<AMC_Chunk>(OutHit.GetActor())){
		NewVoxel->Sides[3] = AddVoxelSide(TEXT("Back"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh);
		NewVoxel->Sides[3]->Parent = NewVoxel;
	} else {

		DrawDebugPoint(GetWorld(), OutHit.Location, 40, FColor::White, true);
		DrawDebugDirectionalArrow(GetWorld(), NewLocation, FVector(NewLocation.X - VoxelSize, NewLocation.Y, NewLocation.Z), 20, FColor::Red, true);
		NewVoxel->Sides[3] = AddVoxelSide(TEXT("Back"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh, false);
		NewVoxel->Sides[3]->Parent = NewVoxel;
		MapSides(NewVoxel->Sides[3], OutHit);
	}

	//Right
	bsuccess = GetWorld()->LineTraceSingleByChannel(OutHit, NewLocation, FVector(NewLocation.X, NewLocation.Y - VoxelSize, NewLocation.Z), ECollisionChannel::ECC_Destructible);
	if(!bsuccess || !Cast<AMC_Chunk>(OutHit.GetActor())){
		NewVoxel->Sides[4] = AddVoxelSide(TEXT("Right"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh);
		NewVoxel->Sides[4]->Parent = NewVoxel;
	} else {
		DrawDebugPoint(GetWorld(), OutHit.Location, 40, FColor::White, true);
		DrawDebugDirectionalArrow(GetWorld(), NewLocation, FVector(NewLocation.X, NewLocation.Y - VoxelSize, NewLocation.Z), 20, FColor::Red, true);
		NewVoxel->Sides[4] = AddVoxelSide(TEXT("Right"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh, false);
		NewVoxel->Sides[4]->Parent = NewVoxel;
		MapSides(NewVoxel->Sides[4], OutHit);
	}

	//Left
	bsuccess = GetWorld()->LineTraceSingleByChannel(OutHit, NewLocation, FVector(NewLocation.X, NewLocation.Y + VoxelSize, NewLocation.Z), ECollisionChannel::ECC_Destructible);
	if(!bsuccess || !Cast<AMC_Chunk>(OutHit.GetActor())){
		NewVoxel->Sides[5] = AddVoxelSide(TEXT("Left"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh);
		NewVoxel->Sides[5]->Parent = NewVoxel;
	} else {
		DrawDebugPoint(GetWorld(), OutHit.Location, 40, FColor::White, true);
		DrawDebugDirectionalArrow(GetWorld(), NewLocation, FVector(NewLocation.X, NewLocation.Y + VoxelSize, NewLocation.Z), 20, FColor::Red, true);
		NewVoxel->Sides[5] = AddVoxelSide(TEXT("Left"), NewLocation, Transform.GetRotation().Rotator(), StaticMesh, false);
		NewVoxel->Sides[5]->Parent = NewVoxel;
		MapSides(NewVoxel->Sides[5], OutHit);
	}
	Voxels.Add(NewVoxel);
}

void AMC_Chunk::RemoveVoxelSide(FSide* Side){

	if(Side == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxelSide: Side is null"));
		return;
	}

	if(!Side->Visible){
		UE_LOG(LogTemp, Warning, TEXT("AMC_Chunk::RemoveVoxelSide: Side no visible"));
		return;
	}

	if(Side->StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxelSide: StaticMesh is null"));
		return;
	}

	//Find index, we are going to trace a small line crossing through the center of the mesh
	FHitResult OutHit;
	FVector end = Side->SideLocation;
	FVector start = FVector(end.X + 1, end.Y + 1, end.Z + 1);


	bool bsuccess = GetWorld()->LineTraceSingleByChannel(OutHit, start, end, ECollisionChannel::ECC_Destructible);
	if(!bsuccess){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxelSide: Didn't hit, trying to find it self"));
		return;
	}

	UInstancedStaticMeshComponent* HitComponent = Cast<UInstancedStaticMeshComponent>(OutHit.GetComponent());
	AMC_Chunk* HitChunk = Cast<AMC_Chunk>(OutHit.GetActor());
	
	if(!HitComponent || !HitChunk){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxelSide: No ACtor or Component"));
		return;
	}

	if(!Side->StaticMesh->RemoveInstance(OutHit.Item)){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxelSide: Not Deleting Instance: %d For for %s"), OutHit.Item, *HitComponent->GetName());
		return;
	}

	//TODO create instance of the side that is besides to this side(To be deleted) and delete this side
	if(Side->Next == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxelSide: Side->Next is null"));
		return;
	}
	FSide* NextSide = Side->Next;
	if(NextSide == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxelSide: NextSide is null "));
		return;
	}

	if(NextSide->Chunk == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxelSide: NextSide->Chunk is null"));
		return;
	}
	NextSide->Chunk->RedrawVoxelSide(NextSide);
}

void AMC_Chunk::RemoveVoxel(FVector Location, UInstancedStaticMeshComponent* StaticMesh){

	if(StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: StaticMesh is null"));
		return;
	}

	if(!Vector2Side.Contains(Location)){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: Missing Location: %s"), *Location.ToString());
		return;
	}
	FSide* DeleteSide = Vector2Side[Location];
	if(DeleteSide == nullptr){
		UE_LOG(LogTemp, Error, TEXT("RemoveVoxel: DeleteSide is nullptr for %s"), *Location.ToString());
		return;
	}

	FVoxel* SideParent = DeleteSide->Parent;
	if(SideParent == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: SideParent is nullptr for %s"), *Location.ToString());
		return;
	}

	for(int i = 5; i >= 0; i--){
		if(SideParent->Sides[i] == nullptr){
			UE_LOG(LogTemp, Warning, TEXT("AMC_Chunk::RemoveVoxel: Side[%d] nullptr "), i);
			continue;
		}
		RemoveVoxelSide(SideParent->Sides[i]);
	}
	delete SideParent;
	if(DeleteSide==nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::RemoveVoxel: Si se esta borrando el pointer"));
	}
}

void AMC_Chunk::MapSides(FSide* Side, FHitResult OutHit){
	if(Side == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::MapSides: Side == nullptr"));
		return;
	}
	// REmove next side from rendering
	UInstancedStaticMeshComponent* HitComponent = Cast<UInstancedStaticMeshComponent>(OutHit.GetComponent());
	AMC_Chunk* HitChunk = Cast<AMC_Chunk>(OutHit.GetActor());
	if(!HitComponent || !HitChunk){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::MapSides: No Actor or Component"));
		return;
	}
	FTransform OutInstanceTransform;
	HitComponent->GetInstanceTransform(OutHit.Item, OutInstanceTransform, false);
	FVector SideCenter = OutInstanceTransform.GetLocation();

	if(!HitChunk->Vector2Side.Contains(SideCenter)){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::MapSides: Vector2Side doens't contain: %s"), *SideCenter.ToString());
		return;
	}
	// Map both sides to each other
	FSide* NextSide = HitChunk->Vector2Side[SideCenter];
	HitChunk->RemoveVoxelSide(NextSide);
	if(NextSide == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AMC_Chunk::MapSides: NextSide == nullptr"));
		return;
	}
	NextSide->Next = Side;
	Side->Next = NextSide;
}
