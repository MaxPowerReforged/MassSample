// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityConfigAsset.h"
#include "MSBoidDevSettings.generated.h"

class AMSBoidNiagaraHelper;
/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta = (DisplayName="Boid Settings"))
class MASSSAMPLE_API UMSBoidDevSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Boids|Assets")
	TSoftObjectPtr<UMassEntityConfigAsset> BoidEntityConfig;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Boids|Assets")
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Boids|Render")
	TSubclassOf<AMSBoidNiagaraHelper> NiagaraActorClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Boids|Render")
	bool UseNiagara = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Boids|Simulation")
	int32 SimulationExtentFromCenter = 40000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Boids|Simulation")
	int32 NumOfBoids = 10;

	/** How many times we divide the Boid array based on the LocationUpdateFrequency */
	UPROPERTY(Config, BlueprintReadWrite, EditAnywhere, Category = "Boids|Net", meta = (ClampMin = 0))
	uint8 BatchesPerUpdate = 10;

	/** Location update frequency in seconds */
	UPROPERTY(Config, BlueprintReadWrite, EditAnywhere, Category = "Boids|Net", meta = (ClampMin = 0))
	float LocationUpdateFrequency = 1.0f;

	/** Update precision tolerance */
	UPROPERTY(Config, BlueprintReadWrite, EditAnywhere, Category = "Boids|Net", meta = (ClampMin = 0))
	float NetUpdatePrecisionTolerance = 2.0f;

	/** How much time we allow orders to arrive (before or after) we received the last update, in seconds */
	UPROPERTY(Config, BlueprintReadWrite, EditAnywhere, Category = "Boids|Net", meta = (ClampMin = 0))
	float NetUpdateTimeThreshold = 0.03f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Boids|Debug")
	bool DrawDebugBoxes = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Boids|Debug")
	bool Static = false;
};
