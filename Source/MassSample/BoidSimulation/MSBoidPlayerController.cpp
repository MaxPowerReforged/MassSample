// Fill out your copyright notice in the Description page of Project Settings.


#include "MSBoidPlayerController.h"

#include "Engine/NetDriver.h"
#include "GameFramework/PlayerState.h"


// Sets default values
AMSBoidPlayerController::AMSBoidPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMSBoidPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetDriver()) GetNetDriver()->bCollectNetStats = true;
}

float AMSBoidPlayerController::GetInBytes()
{
	if (GetWorld() && GetWorld()->GetNetDriver())
	{
		return GetWorld()->GetNetDriver()->InBytesPerSecond;
	}
	return 0.f;
}

float AMSBoidPlayerController::GetOutBytes()
{
	if (GetWorld() && GetWorld()->GetNetDriver())
	{
		return GetWorld()->GetNetDriver()->OutBytesPerSecond;
	}
	return 0.f;
}

float AMSBoidPlayerController::GetPing()
{
	if (GetPlayerState<APlayerState>()) return GetPlayerState<APlayerState>()->ExactPing;
	return 0.f;
}
