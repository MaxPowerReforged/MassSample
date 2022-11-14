// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraActor.h"
#include "GameFramework/Actor.h"
#include "MSBoidNiagaraHelper.generated.h"

UCLASS(Blueprintable)
class MASSSAMPLE_API AMSBoidNiagaraHelper : public ANiagaraActor
{
	GENERATED_BODY()

public:
	AMSBoidNiagaraHelper(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

protected:
	virtual void PostRegisterAllComponents() override;
};
