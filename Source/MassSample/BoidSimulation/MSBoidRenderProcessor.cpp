// Fill out your copyright notice in the Description page of Project Settings.


#include "MSBoidRenderProcessor.h"

#include "MassRepresentationTypes.h"
#include "MSBoidDevSettings.h"
#include "MSBoidFragments.h"
#include "MSBoidMovementProcessor.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

DECLARE_CYCLE_STAT(TEXT("Boids Render ~ HISM update"), STAT_RenderInHism, STATGROUP_BoidsRender);

UMSBoidRenderProcessor::UMSBoidRenderProcessor()
{
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
}

void UMSBoidRenderProcessor::Initialize(UObject& Owner)
{
	BoidSubsystem = GetWorld()->GetSubsystem<UMSBoidSubsystem>();
}

void UMSBoidRenderProcessor::ConfigureQueries()
{
	RenderBoidsQuery.AddRequirement<FMSBoidLocationFragment>(EMassFragmentAccess::ReadOnly);
	RenderBoidsQuery.AddRequirement<FMSBoidVelocityFragment>(EMassFragmentAccess::ReadOnly);
	RenderBoidsQuery.AddRequirement<FMSBoidRenderFragment>(EMassFragmentAccess::ReadOnly);
}

void UMSBoidRenderProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	SCOPE_CYCLE_COUNTER(STAT_RenderInHism);
	const UMSBoidDevSettings* const BoidSettings = GetDefault<UMSBoidDevSettings>();
	TArray<FVector> LocationArray;
	TArray<FVector> RotationArray;
	
	RenderBoidsQuery.ForEachEntityChunk(EntitySubsystem, Context, [this, BoidSettings, &LocationArray, &RotationArray](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		const auto Locations = Context.GetFragmentView<FMSBoidLocationFragment>();
		const auto Velocities = Context.GetFragmentView<FMSBoidVelocityFragment>();
		const auto HismIndexes = Context.GetFragmentView<FMSBoidRenderFragment>();

		for (int i = 0; i < NumEntities; ++i)
		{
			const FVector& Location = Locations[i].Location;
			const FVector& Velocity = Velocities[i].Velocity;
			const uint32 HismIndex = HismIndexes[i].HismId;

			if (BoidSettings->UseNiagara)
			{
				LocationArray.Add(Location);
				RotationArray.Add(Velocity);
			}
			else
			{
				BoidSubsystem->Hism->UpdateInstanceTransform(
					HismIndex,
					FTransform(FRotator(Velocity.X, Velocity.Y, Velocity.Z), Location, FVector(1)),
					//FTransform(Location),
					true,
					false,
					true
				);
			}
		}
	});

	if (BoidSettings->UseNiagara)
	{
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(BoidSubsystem->NiagaraComponent,"MassBoidPositions", LocationArray);
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(BoidSubsystem->NiagaraComponent,"MassBoidRotations", RotationArray);
	}
	else
	{
		if (BoidSubsystem && BoidSubsystem->Hism)
        		BoidSubsystem->Hism->MarkRenderTransformDirty();
	}
}
