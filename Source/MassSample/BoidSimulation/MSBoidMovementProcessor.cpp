// Fill out your copyright notice in the Description page of Project Settings.


#include "MSBoidMovementProcessor.h"

#include "MassCommonTypes.h"
#include "MassRepresentationTypes.h"
#include "MSBoidDevSettings.h"
#include "MSBoidFragments.h"
#include "Kismet/KismetMathLibrary.h"

DECLARE_CYCLE_STAT(TEXT("Boids Move ~ Movement update"), STAT_Move, STATGROUP_BoidsMove);

DECLARE_CYCLE_STAT(TEXT("Boids Move ~ Forces Query"), STAT_MoveForces, STATGROUP_BoidsMove);
DECLARE_CYCLE_STAT(TEXT("Boids Move ~ Query octree"), STAT_QueryOctree, STATGROUP_BoidsMove);
DECLARE_CYCLE_STAT(TEXT("Boids Move ~ Process Octree results"), STAT_ProcessOctreeResults, STATGROUP_BoidsMove);
DECLARE_CYCLE_STAT(TEXT("Boids Move ~ Calculate velocity"), STAT_MoveVelocity, STATGROUP_BoidsMove);
DECLARE_CYCLE_STAT(TEXT("Boids Move ~ Perform Movement"), STAT_MoveMove, STATGROUP_BoidsMove);

UMSBoidMovementProcessor::UMSBoidMovementProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
}

void UMSBoidMovementProcessor::Initialize(UObject& Owner)
{
	BoidSubsystem = GetWorld()->GetSubsystem<UMSBoidSubsystem>();
}

void UMSBoidMovementProcessor::ConfigureQueries()
{
	CalculateForcesQuery.AddRequirement<FMSBoidLocationFragment>(EMassFragmentAccess::ReadOnly);
	CalculateForcesQuery.AddRequirement<FMSBoidForcesFragment>(EMassFragmentAccess::ReadWrite);

	CalculateVelocityQuery.AddRequirement<FMSBoidForcesFragment>(EMassFragmentAccess::ReadOnly);
	CalculateVelocityQuery.AddRequirement<FMSBoidVelocityFragment>(EMassFragmentAccess::ReadWrite);

	MoveBoidsQuery.AddRequirement<FMSBoidVelocityFragment>(EMassFragmentAccess::ReadOnly);
	MoveBoidsQuery.AddRequirement<FMSBoidLocationFragment>(EMassFragmentAccess::ReadWrite);
}

void UMSBoidMovementProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	if (BoidSubsystem->bIsStatic) return;

	SCOPE_CYCLE_COUNTER(STAT_Move);

	CalculateForcesQuery.ForEachEntityChunk(EntitySubsystem, Context, [&, this](FMassExecutionContext& Context)
	{
		SCOPE_CYCLE_COUNTER(STAT_MoveForces);

		const int32 NumEntities = Context.GetNumEntities();
		const float SightRadius = BoidSubsystem->BoidSightRadius;
		const float TargetWeight = BoidSubsystem->TargetWeight;
		const float AlignWeight = BoidSubsystem->AlignWeight;
		const float CohesionWeight = BoidSubsystem->CohesionWeight;
		const float SeparationWeight = BoidSubsystem->SeparationWeight;

		const auto Locations = Context.GetFragmentView<FMSBoidLocationFragment>();
		const auto Forces = Context.GetMutableFragmentView<FMSBoidForcesFragment>();

		FCriticalSection Mutex;
		ParallelFor(NumEntities, [&](int32 i)
		{
			TArray<FMSBoid> BoidsInRadius;

			{
				SCOPE_CYCLE_COUNTER(STAT_QueryOctree);
				const FBoxCenterAndExtent OctreeQuery =
					FBoxCenterAndExtent(Locations[i].Location, FVector(SightRadius));
				BoidsInRadius = BoidSubsystem->GetBoidsInRadius(OctreeQuery);
			}

			{
				SCOPE_CYCLE_COUNTER(STAT_ProcessOctreeResults);
				TArray<FVector> BoidVelocities;
				TArray<FVector> BoidLocations;
				TArray<FVector> BoidRepulsionForces;

				for (const FMSBoid& Boid : BoidsInRadius)
				{
					if (UE::Geometry::DistanceSquared(Locations[i].Location, Boid.Location) < (SightRadius * SightRadius))
					{
						BoidVelocities.Push(Boid.Velocity);
						BoidLocations.Push(Boid.Location);
						FVector RepulsionDirection = Locations[i].Location - Boid.Location;
						//RepulsionDirection = RepulsionDirection * (1 / RepulsionDirection.Length());

						BoidRepulsionForces.Push(RepulsionDirection);
					}
				}

				// const TArray<FMassEntityHandle> BoidsInRadius = BoidSubsystem->GetBoidsInRadius(Locations[i].Location, SightRadius);
				// TArray<FVector> BoidVelocities;
				// TArray<FVector> BoidLocations;
				// TArray<FVector> BoidRepulsionForces;
				//
				// for (const FMassEntityHandle& BoidHandle : BoidsInRadius)
				// {
				// 	const FVector& BoidHandleLocation = EntitySubsystem.GetFragmentDataChecked<FMSBoidLocationFragment>(BoidHandle).Location;
				// 	BoidVelocities.Push(EntitySubsystem.GetFragmentDataChecked<FMSBoidVelocityFragment>(BoidHandle).Velocity);
				// 	BoidLocations.Push(BoidHandleLocation);
				// 	FVector RepulsionDirection = Locations[i].Location - BoidHandleLocation;
				//
				// 	BoidRepulsionForces.Push(RepulsionDirection);
				// }

				const auto TargetForce = FVector::ZeroVector - Locations[i].Location;
				const auto AlignForce = UKismetMathLibrary::GetVectorArrayAverage(BoidVelocities);
				const auto AverageLocations = UKismetMathLibrary::GetVectorArrayAverage(BoidLocations);
				const auto AverageRepulsion = UKismetMathLibrary::GetVectorArrayAverage(BoidRepulsionForces);

				const auto CohesionForce = AverageLocations - Locations[i].Location;

				Mutex.Lock();
				Forces[i].ForceResult = (AlignForce * AlignWeight) + (CohesionForce * CohesionWeight) + (
					AverageRepulsion *
					SeparationWeight) + (TargetForce * TargetWeight);
				Mutex.Unlock();
			}
		});

		for (int i = 0; i < NumEntities; ++i)
		{
			// TArray<FMSBoid> BoidsInRadius;
			//
			// {
			// 	SCOPE_CYCLE_COUNTER(STAT_QueryOctree);
			// 	const FBoxCenterAndExtent OctreeQuery =
			// 		FBoxCenterAndExtent(Locations[i].Location, FVector(SightRadius));
			// 	BoidsInRadius = BoidSubsystem->GetBoidsInRadius(OctreeQuery);
			// }
			//
			// {
			// 	SCOPE_CYCLE_COUNTER(STAT_ProcessOctreeResults);
			// 	TArray<FVector> BoidVelocities;
			// 	TArray<FVector> BoidLocations;
			// 	TArray<FVector> BoidRepulsionForces;
			//
			// 	for (const FMSBoid& Boid : BoidsInRadius)
			// 	{
			// 		if (UE::Geometry::DistanceSquared(Locations[i].Location, Boid.Location) < (SightRadius * SightRadius))
			// 		{
			// 			BoidVelocities.Push(Boid.Velocity);
			// 			BoidLocations.Push(Boid.Location);
			// 			FVector RepulsionDirection = Locations[i].Location - Boid.Location;
			// 			//RepulsionDirection = RepulsionDirection * (1 / RepulsionDirection.Length());
			//
			// 			BoidRepulsionForces.Push(RepulsionDirection);
			// 		}
			// 	}
			//
			// 	// const TArray<FMassEntityHandle> BoidsInRadius = BoidSubsystem->GetBoidsInRadius(Locations[i].Location, SightRadius);
			// 	// TArray<FVector> BoidVelocities;
			// 	// TArray<FVector> BoidLocations;
			// 	// TArray<FVector> BoidRepulsionForces;
			// 	//
			// 	// for (const FMassEntityHandle& BoidHandle : BoidsInRadius)
			// 	// {
			// 	// 	const FVector& BoidHandleLocation = EntitySubsystem.GetFragmentDataChecked<FMSBoidLocationFragment>(BoidHandle).Location;
			// 	// 	BoidVelocities.Push(EntitySubsystem.GetFragmentDataChecked<FMSBoidVelocityFragment>(BoidHandle).Velocity);
			// 	// 	BoidLocations.Push(BoidHandleLocation);
			// 	// 	FVector RepulsionDirection = Locations[i].Location - BoidHandleLocation;
			// 	//
			// 	// 	BoidRepulsionForces.Push(RepulsionDirection);
			// 	// }
			//
			// 	const auto TargetForce = FVector::ZeroVector - Locations[i].Location;
			// 	const auto AlignForce = UKismetMathLibrary::GetVectorArrayAverage(BoidVelocities);
			// 	const auto AverageLocations = UKismetMathLibrary::GetVectorArrayAverage(BoidLocations);
			// 	const auto AverageRepulsion = UKismetMathLibrary::GetVectorArrayAverage(BoidRepulsionForces);
			//
			// 	const auto CohesionForce = AverageLocations - Locations[i].Location;
			//
			// 	Forces[i].ForceResult = (AlignForce * AlignWeight) + (CohesionForce * CohesionWeight) + (
			// 		AverageRepulsion *
			// 		SeparationWeight) + (TargetForce * TargetWeight);
			// }
		}
	});

	CalculateVelocityQuery.ForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		SCOPE_CYCLE_COUNTER(STAT_MoveVelocity);

		const int32 NumEntities = Context.GetNumEntities();
		const auto Velocities = Context.GetMutableFragmentView<FMSBoidVelocityFragment>();
		const auto Forces = Context.GetFragmentView<FMSBoidForcesFragment>();

		for (int i = 0; i < NumEntities; ++i)
		{
			Velocities[i].Velocity = UKismetMathLibrary::ClampVectorSize(
				(Velocities[i].Velocity + Forces[i].ForceResult), 0, BoidSubsystem->BoidMaxSpeed);
		}
	});

	MoveBoidsQuery.ForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		const auto Locations = Context.GetMutableFragmentView<FMSBoidLocationFragment>();
		const auto Velocities = Context.GetFragmentView<FMSBoidVelocityFragment>();

		for (int i = 0; i < NumEntities; ++i)
		{
			SCOPE_CYCLE_COUNTER(STAT_MoveMove);
			Locations[i].Location = Locations[i].Location + Velocities[i].Velocity * Context.GetDeltaTimeSeconds();
		}
	});
}
