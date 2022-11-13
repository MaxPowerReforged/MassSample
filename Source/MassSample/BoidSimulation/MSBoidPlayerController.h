// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MSBoidPlayerController.generated.h"

UCLASS()
class MASSSAMPLE_API AMSBoidPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMSBoidPlayerController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable)
	float GetInBytes();

	UFUNCTION(BlueprintCallable)
	float GetOutBytes();

	UFUNCTION(BlueprintCallable)
	float GetPing();
};
