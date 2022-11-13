// Fill out your copyright notice in the Description page of Project Settings.


#include "MSBoidTrait.h"

#include "MassEntityTemplateRegistry.h"
#include "MSBoidFragments.h"

void UMSBoidTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	BuildContext.AddFragment<FMSBoidLocationFragment>();
	BuildContext.AddFragment<FMSBoidVelocityFragment>();
	BuildContext.AddFragment<FMSBoidForcesFragment>();
	BuildContext.AddFragment<FMSBoidRenderFragment>();
	BuildContext.AddFragment<FMSBoidNetId>();
}
