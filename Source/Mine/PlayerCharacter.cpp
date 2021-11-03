// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "DrawDebugHelpers.h"
#include "MC_Chunk.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APlayerCharacter::LookUp);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("LookRight"), this, &APlayerCharacter::LookRight);
	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed ,this, &APlayerCharacter::Jump);	
	PlayerInputComponent->BindAction(TEXT("Tool"), EInputEvent::IE_Pressed ,this, &APlayerCharacter::Tool);

}

void APlayerCharacter::MoveForward(float AxisValue) 
{
	AddMovementInput(GetActorForwardVector() * AxisValue);
}

void APlayerCharacter::LookUp(float PitchValue)
{
	AddControllerPitchInput(PitchValue);
}

void APlayerCharacter::MoveRight(float AxisValue)
{
	AddMovementInput(GetActorRightVector() * AxisValue);
}

void APlayerCharacter::LookRight(float YawValue) 
{
	AddControllerYawInput( YawValue);
}

void APlayerCharacter::Jump() 
{
	Super::Jump();
}


bool APlayerCharacter::ViewTrace(FHitResult& OutHit, FVector& HitDirection, AController* OwnerController) 
{
	
	FVector outLocation;
	FRotator outRotation;
	OwnerController -> GetPlayerViewPoint(outLocation, outRotation);

	FVector End = outLocation + outRotation.Vector() * MaxRange;
	HitDirection = - outRotation.Vector();
	DrawDebugLine(GetWorld(), outLocation, End , FColor::Red, true);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	return GetWorld()->LineTraceSingleByChannel(OutHit, outLocation, End, ECollisionChannel::ECC_Destructible, Params);
}

AController* APlayerCharacter::GetOwnerController() const
{
	AController* OwnerController = this->GetController();
	return OwnerController;
}


void APlayerCharacter::Tool() 
{
	//call action of cube class
	// For now test the destroy blocks
	
	PutBlock();
	return;
	FHitResult OutHit;
	FVector HitDirection;
	AController* OwnerController = GetOwnerController();
	if(OwnerController == nullptr){
		return;
	}

	bool bSuccess = ViewTrace(OutHit, HitDirection, OwnerController);
	
	if(bSuccess){
		
		DrawDebugPoint(GetWorld(), OutHit.Location, 40, FColor::White, true);

		//Getting instance to be deleted
		UInstancedStaticMeshComponent* HitComponent = Cast<UInstancedStaticMeshComponent>(OutHit.GetComponent());
		AMC_Chunk* HitChunk = Cast<AMC_Chunk>(OutHit.GetActor());
		
		if(!HitComponent || !HitChunk){
			return;
		}
		int32 Item = OutHit.Item;
		FVector VoxelLocation = HitChunk->Instance2Coord[Item];
		FInstancesArray InstanceList = HitChunk->Coord2Array[VoxelLocation];
		for(int32 Instance : InstanceList.array){
			HitComponent->RemoveInstance(Instance);
		}
		
	}
}


void APlayerCharacter::PutBlock(){

	FHitResult OutHit;
	FVector HitDirection;
	AController* OwnerController = GetOwnerController();
	if(OwnerController == nullptr){
		return;
	}

	bool bSuccess = ViewTrace(OutHit, HitDirection, OwnerController);
	
	if(bSuccess){
		
		DrawDebugPoint(GetWorld(), OutHit.Location, 40, FColor::White, true);

		//Getting instance where it hit
		UInstancedStaticMeshComponent* HitComponent = Cast<UInstancedStaticMeshComponent>(OutHit.GetComponent());

		//Get the chunk where it hit
		AMC_Chunk* HitChunk = Cast<AMC_Chunk>(OutHit.GetActor());

		if(!HitComponent || !HitChunk){
			UE_LOG(LogTemp, Warning, TEXT("No ACtor or Component"));
			return;
		}		

		// Material 0
		// TODO: Block Depends on the block selected in the tool bar
		UInstancedStaticMeshComponent* NewBlock = HitChunk->ISMs[0];

		// Get location and Snap location of new voxel
		FTransform Transform;
		
		float VoxelSize = HitChunk->VoxelSize;

		int32 Item = OutHit.Item;

		FVector VoxelLocation = HitChunk->Instance2Coord[Item];
		//FInstancesArray InstanceList = HitChunk->Coord2Array[VoxelLocation];
		//for(int32 Instance : InstanceList.array){
		//	HitComponent->RemoveInstance(Instance);
		//}
		
		FVector HitLocation = OutHit.Location;
		FVector NewLocation = VoxelLocation;
		UE_LOG(LogTemp, Warning, TEXT("VoxelLocation: %s"), *VoxelLocation.ToString());
		UE_LOG(LogTemp, Warning, TEXT("HitLocation: %s"), *HitLocation.ToString());

		float XDiff = VoxelLocation.X - HitLocation.X;
		float YDiff = VoxelLocation.Y - HitLocation.Y;
		float ZDiff = VoxelLocation.Z - HitLocation.Z;
		if( XDiff >= VoxelSize/2 ){
			NewLocation.X = VoxelLocation.X - VoxelSize;
			UE_LOG(LogTemp, Warning, TEXT("Add to X: "));
		} else if( -XDiff >= VoxelSize/2 ){
			NewLocation.X = VoxelLocation.X + VoxelSize;
			UE_LOG(LogTemp, Warning, TEXT("Add to X: "));
		} else if( YDiff >= VoxelSize/2 ){
			NewLocation.Y = VoxelLocation.Y - VoxelSize;
			UE_LOG(LogTemp, Warning, TEXT("Add to Y: "));
		} else if( -YDiff >= VoxelSize/2 ){
			NewLocation.Y = VoxelLocation.Y + VoxelSize;
			UE_LOG(LogTemp, Warning, TEXT("Add to Y: "));
		} else if( ZDiff >= VoxelSize/2 ){
			NewLocation.Z = VoxelLocation.Z - VoxelSize;
			UE_LOG(LogTemp, Warning, TEXT("Add to Z: "));
		} else if( -ZDiff >= VoxelSize/2 ){
			NewLocation.Z = VoxelLocation.Z + VoxelSize;
			UE_LOG(LogTemp, Warning, TEXT("Add to Z: "));
		} else {
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("NewLocation: %s"), *NewLocation.ToString());
		Transform.SetLocation(NewLocation);

		//TODO Check if user is in the range
		
		TArray<int32> instances;
		instances.Init(0, 6);
		instances[0] = HitChunk->AddVoxelSide(TEXT("Top"), Transform.GetLocation(), Transform.GetRotation().Rotator(), NewBlock);
		instances[1] = HitChunk->AddVoxelSide(TEXT("Bottom"), Transform.GetLocation(), Transform.GetRotation().Rotator(), NewBlock);
		instances[2] = HitChunk->AddVoxelSide(TEXT("Front"), Transform.GetLocation(), Transform.GetRotation().Rotator(), NewBlock);
		instances[3] = HitChunk->AddVoxelSide(TEXT("Back"), Transform.GetLocation(), Transform.GetRotation().Rotator(), NewBlock);
		instances[4] = HitChunk->AddVoxelSide(TEXT("Right"), Transform.GetLocation(), Transform.GetRotation().Rotator(), NewBlock);
		instances[5] = HitChunk->AddVoxelSide(TEXT("Left"), Transform.GetLocation(), Transform.GetRotation().Rotator(), NewBlock);

		FInstancesArray ArrayStruct;
		ArrayStruct.array = instances;

		HitChunk->Coord2Array.Add(Transform.GetLocation(), ArrayStruct);

	} else {
		UE_LOG(LogTemp, Warning, TEXT("It didn't hit anything"));
	}
}