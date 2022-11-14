// Fill out your copyright notice in the Description page of Project Settings.


#include "MSBoidNiagaraHelper.h"
#include "MSBoidSubsystem.h"
#include "NiagaraComponent.h"


AMSBoidNiagaraHelper::AMSBoidNiagaraHelper(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMSBoidNiagaraHelper::BeginPlay()
{
	Super::BeginPlay();

	UMSBoidSubsystem* BoidSubsystem = GetWorld()->GetSubsystem<UMSBoidSubsystem>();

	if (BoidSubsystem)
	{
		BoidSubsystem->NiagaraComponent = GetNiagaraComponent();
	}
}

void AMSBoidNiagaraHelper::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	GetNiagaraComponent()->SetTickGroup(ETickingGroup::TG_PostPhysics);
}

