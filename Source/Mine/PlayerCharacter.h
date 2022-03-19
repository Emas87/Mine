// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class MINE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	AController* GetOwnerController() const;
	bool ViewTrace(FHitResult& OutHit, FVector& HitDirection, AController* OwnerController);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Movement
	void MoveForward(float AxisValue);
	void LookUp(float PitchValue);
	void MoveRight(float AxisValue);
	void LookRight(float YawValue);
	void Jump();	
	void Tool();
	void Remove();
	void PutBlock();
	bool DoVectorsCollide(FVector Vector1, FVector Vector2, float Size);


	UPROPERTY(BlueprintReadWrite, Category = "Code")
	int MaxToolRange = 300;

	UPROPERTY(BlueprintReadWrite, Category = "Code")
	float PlayerSpeed = 0.3;
};
