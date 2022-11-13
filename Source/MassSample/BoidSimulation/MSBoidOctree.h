#pragma once

#include "CoreMinimal.h"
#include "Math/GenericOctree.h"
#include "MSBoidOctree.generated.h"

USTRUCT()
struct FMSBoid
{

	GENERATED_BODY()

	UPROPERTY() FVector Location;
	UPROPERTY() FVector Velocity;
	UPROPERTY() uint16 Id;

	FMSBoid() : Location(FVector::ZeroVector), Velocity(FVector::ZeroVector), Id(0)
	{}

	FMSBoid(FVector InLocation, FVector InVelocity, uint16 Id) : Location(InLocation), Velocity(InVelocity), Id(Id)
	{}

	bool operator==(const FMSBoid& Other) const
	{
		return Id == Other.Id;
	}
};

struct FMSBoidOctreeSemantics
{
	// When a leaf gets more than this number of elements, it will split itself into a node with multiple child leaves
	enum { MaxElementsPerLeaf = 10 };

	// This is used for incremental updates.  When removing a polygon, larger values will cause leaves to be removed and collapsed into a parent node.
	enum { MinInclusiveElementsPerNode = 5 };

	// How deep the tree can go.
	enum { MaxNodeDepth = 20 };


	typedef TInlineAllocator<MaxElementsPerLeaf> ElementAllocator;

	FORCEINLINE static FBoxCenterAndExtent GetBoundingBox(const FMSBoid& Element)
	{
		// ignore size, just store a small 0x0x0 box at the actor location. Test if more performant
		return FBoxCenterAndExtent(Element.Location, FVector(0, 0, 0));
	}

	FORCEINLINE static bool AreElementsEqual(const FMSBoid& A, const FMSBoid& B)
	{
		return (A.Id == B.Id);
	}

	FORCEINLINE static void SetElementId(const FMSBoid& Element, FOctreeElementId2 OctreeElementID)
	{}
};

typedef TOctree2<FMSBoid, FMSBoidOctreeSemantics> FMSBoidOctree;
