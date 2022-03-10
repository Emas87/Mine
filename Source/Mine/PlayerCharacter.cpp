// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
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
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APlayerCharacter::LookUp);
	PlayerInputComponent->BindAxis(TEXT("LookRight"), this, &APlayerCharacter::LookRight);
	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed ,this, &APlayerCharacter::Jump);	
	PlayerInputComponent->BindAction(TEXT("Tool"), EInputEvent::IE_Pressed ,this, &APlayerCharacter::Tool);
	PlayerInputComponent->BindAction(TEXT("Remove"), EInputEvent::IE_Pressed ,this, &APlayerCharacter::Remove);

}

void APlayerCharacter::MoveForward(float AxisValue) 
{
	AddMovementInput(GetActorForwardVector() * AxisValue * PlayerSpeed);
}

void APlayerCharacter::LookUp(float PitchValue)
{
	AddControllerPitchInput(PitchValue);
}

void APlayerCharacter::MoveRight(float AxisValue)
{
	AddMovementInput(GetActorRightVector() * AxisValue * PlayerSpeed);
}

void APlayerCharacter::LookRight(float YawValue) 
{
	AddControllerYawInput(YawValue);
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

}

void APlayerCharacter::Remove() 
{
	//call action of cube class
	// For now test the destroy blocks

	FHitResult OutHit;
	FVector HitDirection;
	AController* OwnerController = GetOwnerController();
	if(OwnerController == nullptr){
		UE_LOG(LogTemp, Error, TEXT("APlayerCharacter::Remove: No OwnerController Found"));
		return;
	}

	bool bSuccess = ViewTrace(OutHit, HitDirection, OwnerController);
	
	if(bSuccess){
		
		DrawDebugPoint(GetWorld(), OutHit.Location, 40, FColor::White, true);

		//Getting instance to be deleted
		UInstancedStaticMeshComponent* HitComponent = Cast<UInstancedStaticMeshComponent>(OutHit.GetComponent());
		AMC_Chunk* HitChunk = Cast<AMC_Chunk>(OutHit.GetActor());
		
		if(!HitComponent || !HitChunk){
			UE_LOG(LogTemp, Error, TEXT("APlayerCharacter::Remove: No ACtor or Component"));
			return;
		}

		// FTransform OutInstanceTransform;

		HitChunk->RemoveVoxel(HitComponent, OutHit);
	} else {
		UE_LOG(LogTemp, Warning, TEXT("APlayerCharacter::Remove: It didn't hit anything"));
	}
}

void APlayerCharacter::PutBlock(){

	FHitResult OutHit;
	FVector HitDirection;
	AController* OwnerController = GetOwnerController();
	if(OwnerController == nullptr){
		UE_LOG(LogTemp, Error, TEXT("APlayerCharacter::PutBlock: No OwnerController Found"));
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
			UE_LOG(LogTemp, Error, TEXT("APlayerCharacter::PutBlock: No ACtor or Component"));
			return;
		}

		FTransform OutInstanceTransform;
		HitComponent->GetInstanceTransform(OutHit.Item, OutInstanceTransform, true);
		
		FVector NewLocation = OutInstanceTransform.GetLocation();

		// Get location and Snap location of new voxel
		FTransform Transform;
		float VoxelSize = HitChunk->VoxelSize;
	
		// Hit Location is Global, not local
		FVector HitLocation = OutHit.Location;		
		FVector VoxelLocation = NewLocation;

		// Find greatest difference in axes
		float XDiff = VoxelLocation.X - HitLocation.X;
		float YDiff = VoxelLocation.Y - HitLocation.Y;
		float ZDiff = VoxelLocation.Z - HitLocation.Z;
		if(UKismetMathLibrary::Vector_GetAbsMax(FVector(XDiff, YDiff, ZDiff)) == UKismetMathLibrary::Abs(XDiff)){
			// XDiff is the greatest
			UE_LOG(LogTemp, Error, TEXT("APlayerCharacter::PutBlock: XDiff is the greatest"));	
			if(XDiff > 0){
				NewLocation.X = VoxelLocation.X - VoxelSize;
			} else {
				NewLocation.X = VoxelLocation.X + VoxelSize;
			}
		} else if(UKismetMathLibrary::Vector_GetAbsMax(FVector(XDiff, YDiff, ZDiff)) == UKismetMathLibrary::Abs(YDiff)){
			// YDiff is the greatest
			UE_LOG(LogTemp, Error, TEXT("APlayerCharacter::PutBlock: YDiff is the greatest"));	
			if(YDiff > 0){
				NewLocation.Y = VoxelLocation.Y - VoxelSize;
			} else {
				NewLocation.Y = VoxelLocation.Y + VoxelSize;
			}
		} else if(UKismetMathLibrary::Vector_GetAbsMax(FVector(XDiff, YDiff, ZDiff)) == UKismetMathLibrary::Abs(ZDiff)){
			// ZDiff is the greatest
			UE_LOG(LogTemp, Error, TEXT("APlayerCharacter::PutBlock: ZDiff is the greatest"));	
			if(ZDiff > 0){
				NewLocation.Z = VoxelLocation.Z - VoxelSize;
			} else {
				NewLocation.Z = VoxelLocation.Z + VoxelSize;
			}	
		}

		// if Newlocation doesn't collides with player place new block otherwise ignore block
		if(!DoVectorsCollide(NewLocation, GetActorLocation(), VoxelSize)){
			Transform.SetLocation(NewLocation);
			HitChunk->AddVoxel(Transform, false, 0);
		}

	} else {
		UE_LOG(LogTemp, Warning, TEXT("APlayerCharacter::PutBlock: It didn't hit anything"));
	}
}

bool APlayerCharacter::DoVectorsCollide(FVector Vector1, FVector Vector2, float Size){
	//If vector2 X is out X margin from Vector1
	if(Vector1.X + Size < Vector2.X || Vector1.X - Size > Vector2.X){
		return false;
	}

	//If vector2 Y is in Y margin from Vector1
	if(Vector1.Y + Size < Vector2.Y || Vector1.Y - Size > Vector2.Y){
		return false;
	}

	//If vector2 Z is in Z margin from Vector1
	if(Vector1.Z + Size < Vector2.Z || Vector1.Z - Size > Vector2.Z){
		return false;
	}

	return true;
}