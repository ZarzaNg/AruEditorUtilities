// Copyright Epic Games, Inc. All Rights Reserved.

#include "AruEditorUtilities.h"
#include "MessageLogModule.h"
#include "MessageLogInitializationOptions.h"

#define LOCTEXT_NAMESPACE "FAruEditorUtilitiesModule"

void FAruEditorUtilitiesModule::StartupModule()
{
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	FMessageLogInitializationOptions InitOptions;
	InitOptions.bShowPages = true;
	InitOptions.bAllowClear = true;
	InitOptions.bShowFilters = true;
	InitOptions.bShowInLogWindow = true;
	MessageLogModule.RegisterLogListing("AruEditorUtilitiesModule", NSLOCTEXT("AruEditorUtilitiesModule", "AruEditorLogLabel", "AruEditor Module"), InitOptions);
}

void FAruEditorUtilitiesModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
	{
		FMessageLogModule& MessageLogModule = FModuleManager::GetModuleChecked<FMessageLogModule>("MessageLog");
		MessageLogModule.UnregisterLogListing("AruEditorUtilitiesModule");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAruEditorUtilitiesModule, AruEditorUtilities)