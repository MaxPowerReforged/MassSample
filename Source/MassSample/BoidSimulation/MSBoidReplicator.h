﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MSBoidReplicator.generated.h"

class UMSBoidSubsystem;
struct FMSBoid;

USTRUCT()
struct FMSBoidLocationNet
{
	GENERATED_BODY()
	
	UPROPERTY() uint16 LocationX;
	UPROPERTY() uint16 LocationY;
	UPROPERTY() uint16 LocationZ;
	UPROPERTY() uint16 BoidId;

	FMSBoidLocationNet()
	{
		LocationX = 0;
		LocationY = 0;
		LocationZ = 0;
		BoidId = 0;
	}

	FMSBoidLocationNet(uint16 LocationX, uint16 LocationY, uint16 LocationZ, uint16 BoidId)
	{
		this->LocationX = LocationX;
		this->LocationY = LocationY;
		this->LocationZ = LocationZ;
		this->BoidId = BoidId;
	}
};

USTRUCT()
struct FMSBoidCachedLocation
{
	GENERATED_BODY()
	
	UPROPERTY() float LocationX;
	UPROPERTY() float LocationY;
	UPROPERTY() float LocationZ;

	FMSBoidCachedLocation()
	{
		LocationX = 0;
		LocationY = 0;
		LocationZ = 0;
	}

	FMSBoidCachedLocation(float LocationX, float LocationY, float LocationZ)
	{
		this->LocationX = LocationX;
		this->LocationY = LocationY;
		this->LocationZ = LocationZ;
	}
};

UCLASS()
class MASSSAMPLE_API AMSBoidReplicator : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMSBoidReplicator();

	/** Server-side. Multicast Boids' locations every N seconds */
	UFUNCTION() void CheckLocations();

	/** All clients-side. Apply server's Boid location */
	UFUNCTION(NetMulticast, Unreliable)
	void NetCastLocations(const TArray<FMSBoidLocationNet>& BoidLocations, int32 ServerStepNumber);

	UFUNCTION()
	void StartUpdates();

	UFUNCTION()
	void StopUpdates();
	
	void AddBoid(const FMSBoid& Boid);
	
	void RemoveBoid(const FMSBoid& Boid);

	/** Checks update timings to see if it was not in time and therefore has outdated positions */
	UFUNCTION()
	bool IsUpdateValid();
	
	TArray<FMSBoid> Boids;

	/** Stores past boid Locations to check if they have changed */
	//TMap<FMSBoid, FMSBoidCachedLocation> CachedBoidLocations;

	/** How many times we divide the Boid array based on the LocationUpdateFrequency */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Net Testing", meta = (ClampMin = 0))
	uint8 BatchesPerUpdate = 10;

	/** Location update frequency in seconds */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Net Testing", meta = (ClampMin = 0))
	float LocationUpdateFrequency = 1.0f;

	/** Update precision tolerance */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Net Testing", meta = (ClampMin = 0))
	float NetUpdatePrecisionTolerance = 2.0f;

	/** How much time we allow orders to arrive (before or after) we received the last update, in seconds */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Net Testing", meta = (ClampMin = 0))
	float NetUpdateTimeThreshold = 0.03f;

	UPROPERTY()
	UMSBoidSubsystem* BoidSubsystem;

	UPROPERTY()
	TArray<FMSBoidLocationNet> BoidsToUpdate;

	uint8 CurrentBatchIndex = 0;
	int CurrentBoidIndex = 0;
	uint16 BoidsPerBatch = 0;
	float LastUpdateTime = 0;
	FTimerHandle UpdateTimerHandle;
	UPROPERTY() int32 StepNumber = 0;
};
