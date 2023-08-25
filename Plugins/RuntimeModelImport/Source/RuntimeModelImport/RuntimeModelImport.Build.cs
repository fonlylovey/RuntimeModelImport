// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.IO;

public class RuntimeModelImport : ModuleRules
{
	public RuntimeModelImport(ReadOnlyTargetRules Target) : base(Target)
    {
        bLegacyPublicIncludePaths = false;
		bEnableExceptions = true;
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
                
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "FBX",
                "RuntimeMeshComponent",
				// ... add other public dependencies that you statically link with here ...
			}
			);

        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
                "SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
            );
        /*
        string FBXSDKDir = Target.UEThirdPartySourceDirectory + "FBX/2020.2/";
        PublicSystemIncludePaths.AddRange(
            new string[] {
                    FBXSDKDir + "include",
                    FBXSDKDir + "include/fbxsdk",
                }
            );
        
        string FBXDLLDir = Target.UEThirdPartyBinariesDirectory + "FBX/2020.2/";
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string FBxLibPath = FBXSDKDir + "lib/vs2017/";
            string FBxDllPath = FBXDLLDir + "Win64/libfbxsdk.dll";

            FBxLibPath += "x64/release/";
            PublicAdditionalLibraries.Add(FBxLibPath + "libfbxsdk.lib");

            // We are using DLL versions of the FBX libraries
            PublicDefinitions.Add("FBXSDK_SHARED");

            RuntimeDependencies.Add("$(TargetOutputDir)/libfbxsdk.dll", FBxDllPath);

            if (Target.LinkType != TargetLinkType.Monolithic)
            {
                
            }
          
        }
        */
    }
}
