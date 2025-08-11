#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "HAL/FileManager.h"
#include "HighResScreenshot.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"

#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#include "Android/AndroidJavaEnv.h"
#endif

#include "ScreenshotLibrary.generated.h"

UCLASS()
class UScreenshotLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Take screenshot, save with timestamp, and move when completed */
    UFUNCTION(BlueprintCallable, Category = "Platform|Screenshot")
    static void TakeCustomScreenshot(const FString& BaseFileName, const FString& SubFolder, int32 Width, int32 Height);
};

void UScreenshotLibrary::TakeCustomScreenshot(const FString& BaseFileName, const FString& SubFolder, int32 Width, int32 Height)
{
    if (!GEngine || !GEngine->GameViewport) return;

    // Filename with timestamp
    FString Timestamp = FDateTime::Now().ToString(TEXT("%Y_%m_%d-%H_%M_%S"));
    FString FileName = FString::Printf(TEXT("%s_%s.png"), *BaseFileName, *Timestamp);

    FString ScreenshotDir;
#if PLATFORM_ANDROID
    ScreenshotDir = FPaths::ProjectSavedDir() / TEXT("Screenshots/Android/");
#elif PLATFORM_WINDOWS
    ScreenshotDir = FPaths::ProjectSavedDir() / TEXT("Screenshots/Windows/");
#else
    ScreenshotDir = FPaths::ProjectSavedDir() / TEXT("Screenshots/");
#endif
    
    // Custom resolution
    FHighResScreenshotConfig& HighResConfig = GetHighResScreenshotConfig();
    HighResConfig.bDateTimeBasedNaming = true;
    HighResConfig.FilenameOverride = FileName;
    if (Width > 0 && Height > 0)
        HighResConfig.SetResolution(Width, Height, 1.0f);
    else
        HighResConfig.SetResolution(GSystemResolution.ResX, GSystemResolution.ResY, 1.0f);

    // Request screenshot
    FScreenshotRequest::RequestScreenshot(ScreenshotDir, false, false);
}
