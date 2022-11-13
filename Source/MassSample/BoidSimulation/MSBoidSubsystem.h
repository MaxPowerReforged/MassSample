// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityConfigAsset.h"
#include "MassEntitySubsystem.h"
#include "MSBoidOctree.h"
#include "MSBoidReplicator.h"
#include "Common/Fragments/MSHashGridFragments.h"
#include "MSBoidSubsystem.generated.h"

class UMSBoidDevSettings;
class UMassEntitySubsystem;
/**
 * 
 */
UCLASS()
class MASSSAMPLE_API UMSBoidSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

public:
	TArray<FMSBoid> GetBoidsInRadius(const FBoxCenterAndExtent& QueryBox);
	TArray<FMassEntityHandle> GetBoidsInRadius(FVector Center, float Radius);

	void SpawnBoidsFromData(const TArray<FMSBoidNetSpawnData>& NewBoidData);

	UFUNCTION(BlueprintCallable)
	void SpawnRandomBoids();

	UPROPERTY(Transient)
	UMassEntitySubsystem* MassEntitySubsystem;

	UPROPERTY()
	UMassEntityConfigAsset* BoidEntityConfig;
	FMassExecutionContext Context;

	UPROPERTY()
	AMSBoidReplicator* BoidReplicator;
	
	TUniquePtr<FMSBoidOctree> BoidOctree;

	FMSHashGrid3D HashGrid = FMSHashGrid3D(100.0f,FMassEntityHandle());

	UPROPERTY()
	UHierarchicalInstancedStaticMeshComponent* Hism;

	UPROPERTY()
	UMSBoidDevSettings* BoidSettings;

	UPROPERTY()
	TMap<uint16, FMassEntityHandle> NetIdMassHandleMap;
	
	void DrawDebugOctree();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boids|Simulation")
	int32 BoidMaxSpeed = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boids|Simulation")
	float BoidSightRadius = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boids|Forces")
	float TargetWeight = 0.001;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boids|Forces")
	float AlignWeight = 0.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boids|Forces")
	float SeparationWeight = 0.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boids|Forces")
	float CohesionWeight = 0.5;

private:
	FMSBoidNetSpawnData GenerateBoidRandomData();

	int32 SimulationExtentFromCenter;
	int32 NumOfBoids;

	// just for testing, incremental boid Id to map boids to FMassEntityHandles across the net
	uint16 NextBoidId = 0;

public:
	bool bDrawDebugBoxes;
	bool bIsStatic;
};
