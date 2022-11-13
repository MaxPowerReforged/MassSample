// Fill out your copyright notice in the Description page of Project Settings.


#include "MSBoidSubsystem.h"

#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MSBoidDevSettings.h"
#include "MSBoidFragments.h"
#include "MSBoidHismHelper.h"
#include "Common/Misc/MSBPFunctionLibrary.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "GameFramework/GameStateBase.h"

void UMSBoidSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BoidSettings = GetMutableDefault<UMSBoidDevSettings>();

	BoidEntityConfig = BoidSettings->BoidEntityConfig.LoadSynchronous();
	SimulationExtentFromCenter = BoidSettings->SimulationExtentFromCenter;
	NumOfBoids = BoidSettings->NumOfBoids;
	bDrawDebugBoxes = BoidSettings->DrawDebugBoxes;
	bIsStatic = BoidSettings->Static;

	BoidReplicator = (AMSBoidReplicator*)GetWorld()->SpawnActor(AMSBoidReplicator::StaticClass());
	BoidReplicator->BoidSubsystem = this;
	BoidReplicator->BatchesPerUpdate = BoidSettings->BatchesPerUpdate;
	BoidReplicator->LocationUpdateFrequency = BoidSettings->LocationUpdateFrequency;
	BoidReplicator->NetUpdatePrecisionTolerance = BoidSettings->NetUpdatePrecisionTolerance;
	BoidReplicator->NetUpdateTimeThreshold = BoidSettings->NetUpdateTimeThreshold;


	BoidOctree = MakeUnique<FMSBoidOctree>(FVector::ZeroVector, SimulationExtentFromCenter);

	GetWorld()->SpawnActor<AMSBoidHismHelper>(AMSBoidHismHelper::StaticClass());

	MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();

	Context = MassEntitySubsystem->CreateExecutionContext(0);
}

void UMSBoidSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	BoidReplicator->StartUpdates();
}

TArray<FMSBoid> UMSBoidSubsystem::GetBoidsInRadius(const FBoxCenterAndExtent& QueryBox)
{
	TArray<FMSBoid> FoundBoids;
	BoidOctree->FindElementsWithBoundsTest(QueryBox, [&](const FMSBoid& Boid)
	{
		FoundBoids.Push(Boid);
	});

	return FoundBoids;
}

TArray<FMassEntityHandle> UMSBoidSubsystem::GetBoidsInRadius(FVector Center, float Radius)
{
	TArray<FMassEntityHandle> FoundBoids;
	HashGrid.FindPointsInBall(Center, Radius, [&, Center](const FMassEntityHandle& Entity)
	{
		const FVector EntityLocation = MassEntitySubsystem->GetFragmentDataChecked<FMSBoidLocationFragment>(Entity).
		                                                    Location;
		return UE::Geometry::DistanceSquared(Center, EntityLocation);
	}, FoundBoids);

	return FoundBoids;
}

void UMSBoidSubsystem::DrawDebugOctree()
{
	if (bDrawDebugBoxes)
	{
		// DrawDebugBox(
		// 	GetWorld(),
		// 	BoidOctree->GetRootBounds().GetBox().GetCenter(),
		// 	BoidOctree->GetRootBounds().GetBox().GetExtent(),
		// 	FColor::Green,
		// 	false,
		// 	-1,
		// 	0,
		// 	30
		// );

		BoidOctree->FindNodesWithPredicate(
			[](FMSBoidOctree::FNodeIndex ParentNodeIndex, FMSBoidOctree::FNodeIndex CurrentNodeIndex,
			   const FBoxCenterAndExtent& Bounds)
			{
				return true;
			},
			[&](FMSBoidOctree::FNodeIndex ParentNodeIndex, FMSBoidOctree::FNodeIndex CurrentNodeIndex,
			    const FBoxCenterAndExtent& Bounds)
			{
				// debug to generate rand color for each node
				// auto RandVector = FRandomStream(CurrentNodeIndex).VRand();
				// FColor RandColor = FColor(RandVector.X * 255, RandVector.Y * 255, RandVector.Z * 255, 1);

				DrawDebugBox(
					GetWorld(),
					Bounds.GetBox().GetCenter(),
					Bounds.GetBox().GetExtent(),
					FColor::Cyan,
					false,
					-1,
					0,
					8
				);

				TArrayView<const FMSBoid> BoidsInNode = BoidOctree->GetElementsForNode(
					CurrentNodeIndex);
				for (const FMSBoid& Boid : BoidsInNode)
				{
					DrawDebugSphere(
						GetWorld(),
						Boid.Location,
						100,
						8,
						FColor::Red,
						false,
						-1,
						0,
						8
					);
				}
			});
	}
}

void UMSBoidSubsystem::SpawnRandomBoids()
{
	// the random data is only generated on the server to be later multicasted to clients
	if (GetWorld()->GetNetMode() == ENetMode::NM_Client) return;

	TArray<FMSBoidNetSpawnData> SpawnData;

	for (int i = 0; i < NumOfBoids; ++i)
	{
		SpawnData.Push(GenerateBoidRandomData());
	}

	BoidReplicator->NetCastSpawnBoids(SpawnData);
}

FMSBoidNetSpawnData UMSBoidSubsystem::GenerateBoidRandomData()
{
	NextBoidId++;
	return FMSBoidNetSpawnData(
		NextBoidId,
		FMath::VRand() * FMath::RandRange(10, SimulationExtentFromCenter / 2),
		FMath::VRand() * FMath::RandRange(10, BoidMaxSpeed)
	);
}

void UMSBoidSubsystem::SpawnBoidsFromData(const TArray<FMSBoidNetSpawnData>& NewBoidData)
{
	if (GetWorld()->GetNetMode() == ENetMode::NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMSBoidSubsystem::SpawnBoidsFromData() in client num: %d"), NewBoidData.Num());
	}
	
	if (!BoidEntityConfig) return;

	for (const FMSBoidNetSpawnData& BoidData : NewBoidData)
	{
		FEntityHandleWrapper NewBoid = UMSBPFunctionLibrary::SpawnEntityFromEntityConfig(
			BoidEntityConfig,
			GetWorld(),
			false
		);

		MassEntitySubsystem->GetFragmentDataChecked<FMSBoidLocationFragment>(NewBoid.Entity).Location = BoidData.Location;
		MassEntitySubsystem->GetFragmentDataChecked<FMSBoidVelocityFragment>(NewBoid.Entity).Velocity = BoidData.Velocity;

		int32 HismIndex = Hism->AddInstance(FTransform(BoidData.Location), true);
		MassEntitySubsystem->GetFragmentDataChecked<FMSBoidRenderFragment>(NewBoid.Entity).HismId = HismIndex;

		MassEntitySubsystem->GetFragmentDataChecked<FMSBoidNetId>(NewBoid.Entity).Id = BoidData.NetId;

		FMSBoid NewBoidStruct = FMSBoid(
			BoidData.Location,
			BoidData.Velocity,
			BoidData.NetId
		);

		NetIdMassHandleMap.Add(BoidData.NetId, NewBoid.Entity);
		
		// HashGrid.InsertPoint(NewBoid.Entity, BoidData.Location);

		if (bDrawDebugBoxes) UE_LOG(LogTemp, Warning, TEXT("UMSBoidSubsystem::SpawnBoid() id: %d, location: %s"),
							NewBoid.Entity.Index, *BoidData.Location.ToString());

		// only add to replicator if server
		if (GetWorld()->GetNetMode() == ENetMode::NM_Client) continue;

		BoidReplicator->AddBoid(NewBoidStruct);
	}
}
