// Fill out your copyright notice in the Description page of Project Settings.

#include "MC_Chunk.h"
#include "DrawDebugHelpers.h"

AMC_Chunk::AMC_Chunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	TArray<UInstancedStaticMeshComponent*> 
	foreach(ISM :ISMs){
		Indexes.Add(ISM, new TDoubleLinkedList<int32>);
		TMap<UInstancedStaticMeshComponent,TDoubleLinkedList<int32>> Indexes;
	}
}

AMC_Chunk::~AMC_Chunk()
{
	UE_LOG(LogTemp, Warning, TEXT("Size Voxels: %d"), Voxels.GetAllocatedSize());
 	/*for(int i = Voxels.Num() - 1; i >= 0 ; i--){
		 delete Voxels[i];
	}*/
	UE_LOG(LogTemp, Warning, TEXT("Size Voxels: %d"), Voxels.GetAllocatedSize());
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
		UE_LOG(LogTemp, Error, TEXT("RedrawVoxelSide: StaticMesh is null"));
		return;
	}

	if(Side->StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("RedrawVoxelSide: Side->StaticMesh not initialized"));
		return;
	}

	if(Side->InstanceIndex == nullptr){
		UE_LOG(LogTemp, Error, TEXT("RedrawVoxelSide: Already Redrawn"));
		return;
	}

	FTransform Transform;
	int32 InstanceNumber;

	Transform.SetLocation(Side->SideCenter);
	Transform.SetRotation(FQuat(Side->SideRotation));
	InstanceNumber = Side->StaticMesh->AddInstance(Transform);

	if(!Indexes.Contains(Side->StaticMesh)){
		UE_LOG(LogTemp, Error, TEXT("RedrawVoxelSide: Indexes doesn't contain staticmesh: %s"), *Side->StaticMesh->GetName());
		return ;
	}

	//Adding new node and then make NewSide to point to that Node
	TDoubleLinkedList<int32>* List = Indexes[Side->StaticMesh];
	List->AddTail(InstanceNumber);
	Side->InstanceIndex = &(List->GetTail());

	Instance2Side.Add(FString::FromInt(Side->InstanceIndex->GetVaue()) + " " + Side->StaticMesh->GetName() , Side);
}

FSide* AMC_Chunk::AddVoxelSide(FString Side, FVector Location, FRotator Rotation, UInstancedStaticMeshComponent* StaticMesh, bool AddInstance){
	FSide* NewSide = new FSide;
	if(StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AddVoxelSide: StaticMesh is null"));
		return NewSide;
	}
	FTransform Transform;
	int32 InstanceNumber = -1;
	if(Side == TEXT("Top")){
		Transform.SetLocation(FVector(Location.X, Location.Y, Location.Z + VoxelSize/2));
		Transform.SetRotation(FQuat(Rotation));
		InstanceNumber = StaticMesh->AddInstance(Transform);

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
		UE_LOG(LogTemp, Error, TEXT("Trying a different side %s"), *Side);
		return NewSide;
	}

	if(AddInstance){
		InstanceNumber = StaticMesh->AddInstance(Transform);
	}

	if(Instance2Side.Contains(FString::FromInt(InstanceNumber) + " " + StaticMesh->GetName())){
		UE_LOG(LogTemp, Error, TEXT("AddVoxelSide: Instance2Side Already contains: %d for %s in Location %s, and current location is %s"),
		 InstanceNumber, *Side, *(Instance2Side[FString::FromInt(InstanceNumber) + " " + StaticMesh->GetName()])->VoxelCenter.ToString() , *(Location).ToString());
		return NewSide;
	}

	if(!Indexes.Contains(StaticMesh)){
		UE_LOG(LogTemp, Error, TEXT("AddVoxelSide: Indexes doesn't contain staticmesh: %s"), *StaticMesh->GetName());
		return NewSide;
	}
	
	//Adding new node and then make NewSide to point to that Node
	TDoubleLinkedList<int32>* List = Indexes[StaticMesh];
	List->AddTail(InstanceNumber);
	NewSide->InstanceIndex = &(List->GetTail());

	NewSide->Name = Side;
	NewSide->VoxelCenter = Location;
	NewSide->SideCenter = Transform.GetLocation();
	NewSide->SideRotation = Transform.GetRotation();
	NewSide->StaticMesh = StaticMesh;
	NewSide->Chunk = this;
	Instance2Side.Add(FString::FromInt(InstanceNumber) + " " + StaticMesh->GetName() , NewSide);
	//return FString::FromInt(InstanceNumber) + " " + StaticMesh->GetName();
	return NewSide;
}

void AMC_Chunk::AddVoxel(FTransform Transform, UInstancedStaticMeshComponent* StaticMesh){

	if(StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("AddVoxel: StaticMesh is null"));
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
	if(NewVoxel->Sides[0] != nullptr){
		UE_LOG(LogTemp, Warning, TEXT("%s"),*NewVoxel->Sides[0]->ToString());
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

void AMC_Chunk::RemoveVoxelSide(FString IndexComponent, UInstancedStaticMeshComponent* StaticMesh){

	if(StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("RemoveVoxelSide: StaticMesh is null"));
		return;
	}
	FString Instance;
	FString Component;
	if(!IndexComponent.Split(" ", &Instance, &Component)){
		UE_LOG(LogTemp, Error, TEXT("RemoveVoxelSide: Couldn't Split %s"), *IndexComponent);
		return;
	}

	if(!StaticMesh->RemoveInstance(FCString::Atoi(*Instance))){
		UE_LOG(LogTemp, Error, TEXT("RemoveVoxelSide: Not Deleting Instance: %d For for %s"), FCString::Atoi(*Instance), *Component);
		return;
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("Deleting Instance: %d For for %s"), FCString::Atoi(*Instance), *Component)
	if(!Instance2Side.Contains(IndexComponent)){
		UE_LOG(LogTemp, Error, TEXT("RemoveVoxelSide: Missing Instance and Component Combination: %s"), *IndexComponent);
		return;
	}
	FSide* Side = Instance2Side[IndexComponent];
	if(Side == nullptr){
		UE_LOG(LogTemp, Error, TEXT("RemoveVoxelSide: Side == nullptr"), *IndexComponent);
		return;
	}

	//Deleting node and then make NewSide to point to that Node
	if(!Indexes.Contains(StaticMesh)){
		UE_LOG(LogTemp, Error, TEXT("AddVoxelSide: Indexes doesn't contain staticmesh: %s"), *StaticMesh->GetName());
		return NewSide;
	}
	
	RemoveNodeFromList(Side);

	//TODO Verify if Side->InstanceIndex point to nullptr after removing node
	if(Side->InstanceIndex == nullptr){
		UE_LOG(LogTemp, Error, TEXT("Si Se Borro solo"), *StaticMesh->GetName());
	} else {
		Side->InstanceIndex = nullptr;
	}
	
	Instance2Side.Remove(IndexComponent);

	//TODO create instance of the side that is besides to this side(To be deleted) and delete this side
	if(Side->Next == nullptr){
		UE_LOG(LogTemp, Error, TEXT("RemoveVoxelSide: Side->Next is null"));
		return;
	}
	FSide* NextSide = Side->Next;
	if(NextSide == nullptr){
		UE_LOG(LogTemp, Error, TEXT("RemoveVoxelSide: NextSide is null "));
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("%s"),*(NextSide->ToString()));

	if(NextSide->Chunk == nullptr){
		UE_LOG(LogTemp, Error, TEXT("RemoveVoxelSide: NextSide->Chunk is null"));
		return;
	}
	NextSide->Chunk->RedrawVoxelSide(NextSide);
}

void AMC_Chunk::RemoveVoxel(int32 index, UInstancedStaticMeshComponent* StaticMesh){

	if(StaticMesh == nullptr){
		UE_LOG(LogTemp, Error, TEXT("RemoveVoxel: StaticMesh is null"));
		return;
	}

	FString IndexComponent = FString::FromInt(index) + " " + StaticMesh->GetName();
	
	if(!Instance2Side.Contains(IndexComponent)){
		UE_LOG(LogTemp, Error, TEXT("Missing Instance and Component Combination: %s"), *IndexComponent);
		return;
	}
	FSide* DeleteSide = Instance2Side[IndexComponent];
	FVoxel* SideParent = DeleteSide->Parent;

	if(SideParent == nullptr){
		UE_LOG(LogTemp, Warning, TEXT("RemoveVoxel: SideParent is nullptr for %s"), *IndexComponent);
		return;
	}

	for(int i = 5; i >= 0; i--){
		if(SideParent->Sides[i] == nullptr){
			UE_LOG(LogTemp, Warning, TEXT("RemoveVoxel: Side[%d] nullptr "), i);
			continue;
		}
		IndexComponent = FString::FromInt(SideParent->Sides[i]->InstanceIndex->GetValue()) + " " + StaticMesh->GetName();	
		RemoveVoxelSide(IndexComponent, StaticMesh);
	}
	delete SideParent;
	if(DeleteSide==nullptr){
		UE_LOG(LogTemp, Error, TEXT("Si se esta borrando el pointer"));
	}
}

void AMC_Chunk::MapSides(FSide* Side, FHitResult OutHit){
	if(Side == nullptr){
		UE_LOG(LogTemp, Error, TEXT("MapSides: Side == nullptr"));
		return;
	}
	// REmove next side from rendering
	UInstancedStaticMeshComponent* HitComponent = Cast<UInstancedStaticMeshComponent>(OutHit.GetComponent());
	AMC_Chunk* HitChunk = Cast<AMC_Chunk>(OutHit.GetActor());
	if(!HitComponent || !HitChunk){
		UE_LOG(LogTemp, Error, TEXT("MapSides: No Actor or Component"));
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
		UE_LOG(LogTemp, Error, TEXT("MapSides: NextSide == nullptr"));
		return;
	}
	NextSide->Next = Side;
	Side->Next = NextSide;
}


void AMC_Chunk::RemoveNodeFromList(FSide* Side){
	// TODO Cuando borro un instance todos los indices cambian, necesito almacenar los indices y cuando cambien cambien los que estan despues del que borre
	// Reassign Indexes and Instance2Side;
	TDoubleLinkedList<int32>* List = Indexes[StaticMesh];
	TDoubleLinkedListNode<int32>* InstanceIndex = Side->InstanceIndex;

	List->RemoveNode(InstanceIndex);

}