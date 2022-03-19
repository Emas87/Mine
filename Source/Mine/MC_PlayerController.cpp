// Fill out your copyright notice in the Description page of Project Settings.


#include "MC_PlayerController.h"


void AMC_PlayerController::BeginPlay()
{
	Super::BeginPlay();
    // data tables to store terrain
    // point data tables after file is created
    CreateChunkDataTableFile(true);
    CreateVoxelDataTableFile(true);
    
    //FFileHelper::SaveStringToFile(DataTable->GetTableAsCSV(EDataTableExportFlags::UsePrettyPropertyNames | EDataTableExportFlags::UsePrettyEnumNames), *OutFilenames[0]);

    // init values
    ChunkSizeXY = ChunkWith * 2 * ChunkVoxelSize;
}

bool AMC_PlayerController::CreateChunkDataTableFile(bool New){
    FString filePath = DataDirectory + ChunkDataTableName + TEXT(".csv");
    if(!FileManager.FileExists(*filePath)){
        UE_LOG(LogTemp, Warning, TEXT("Archivo no existe"));
        //Write the columns into the file
        IFileHandle * fileHandler = FileManager.OpenWrite(*filePath, true, true);
        delete fileHandler;
        FFileHelper::SaveStringToFile(TEXT("ChunkLocation\n"), *filePath);
    } else if(New){
        UE_LOG(LogTemp, Warning, TEXT("csv shouldn't exist: %s"), *filePath);
        return false;
    }
    // Define DataTable
    ChunkDataTable = NewObject<UDataTable>(GetTransientPackage(), FName(TEXT("ChunkTable")));
    ChunkDataTable->RowStruct = FChunkDataTableRow::StaticStruct();

    //load csv data into datatable
    FString dataTableString;
    FFileHelper::LoadFileToString(dataTableString, *filePath);
    ChunkDataTable->CreateTableFromCSVString(*dataTableString);

    // This is how to add rows and store datatable into csv file
    /*FChunkDataTableRow temp;
    temp.ChunkLocation = FVector(2,2,3);
    ChunkDataTable->AddRow(FName(temp.ChunkLocation.ToString()),temp);
    FFileHelper::SaveStringToFile(ChunkDataTable->GetTableAsCSV(EDataTableExportFlags::None), *filePath);*/
    return true;
}

bool AMC_PlayerController::CreateVoxelDataTableFile(bool New){
    FString filePath = DataDirectory + VoxelDataTableName + TEXT(".csv");
    if(!FileManager.FileExists(*filePath)){
        //Write the columns into the file
        IFileHandle * fileHandler = FileManager.OpenWrite(*filePath, true, true);
        delete fileHandler;
        FFileHelper::SaveStringToFile(TEXT("InstanceIndex,ChunkLocation\n"), *filePath);
    } else if(New){
        UE_LOG(LogTemp, Warning, TEXT("csv shouldn't exist: %s"), *filePath);
        return false;
    }
    // Define DataTable
    VoxelDataTable = NewObject<UDataTable>(GetTransientPackage(), FName(TEXT("VoxelTable")));
    VoxelDataTable->RowStruct = FVoxelDataTableRow::StaticStruct();

    //load csv data into datatable
    FString dataTableString;
    FFileHelper::LoadFileToString(dataTableString, *filePath);
    VoxelDataTable->CreateTableFromCSVString(*dataTableString);

    return true;
}

void AMC_PlayerController::GenerateTerrain(){
    //CreateChunkDataTableFile();
    //CreateVoxelDataTableFile();
    //Get chunk coordinates
    FVector PlayerLocation = GetPlayerChunkCoordinates();
    for(int i=-ChunkRange/2; i < ChunkRange/2; i++){
        for(int j=-ChunkRange/2; j < ChunkRange/2; j++){
            //check if it is alrady in chunk file otherwise store it
            if(ChunkCoordinates.Contains(FVector(PlayerLocation.X, PlayerLocation.Y, 0))){
                continue;
            }
            ChunkCoordinates.Add(FVector(PlayerLocation.X, PlayerLocation.Y, 0));
            //store chunk coordinates in the csv file
            //build chunk
            //store each voxel in csv file

        }
    }
}

FVector AMC_PlayerController::GetPlayerChunkCoordinates(){
    FVector PlayerLocation = GetPawn()->GetActorLocation();
    PlayerLocation = PlayerLocation/ChunkSizeXY;
    PlayerLocation = FVector(round(PlayerLocation.X),round(PlayerLocation.Y),round(PlayerLocation.Z));
    return PlayerLocation;

}


