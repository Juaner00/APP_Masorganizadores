#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "HighResScreenshot.h"
#include "HAL/FileManager.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "TimerManager.h"
#include "Slate/SceneViewport.h"

#if PLATFORM_ANDROID
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
    static void TakeCustomScreenshot(const FString& BaseFileName, const FString& SubFolder, int32 Width, int32 Height, float TimeoutSeconds = 5.0f);
};

void UScreenshotLibrary::TakeCustomScreenshot(const FString& BaseFileName, const FString& SubFolder, int32 Width, int32 Height, float TimeoutSeconds)
{
    if (!GEngine || !GEngine->GameViewport) return;

    // Filename with timestamp
    FString Timestamp = FDateTime::Now().ToString(TEXT("%Y_%m_%d-%H_%M_%S"));
    FString FileName = FString::Printf(TEXT("%s_%s.png"), *BaseFileName, *Timestamp);

    // // Get actual default screenshot directory for current platform/mode
    // FString ScreenshotDir = FPaths::ConvertRelativePathToFull(FPaths::ScreenShotDir());
    // IFileManager::Get().MakeDirectory(*ScreenshotDir, true);
    //
    // FString TempScreenshotPath = ScreenshotDir / FileName;

    // Set resolution
    FHighResScreenshotConfig& HighResConfig = GetHighResScreenshotConfig();
    HighResConfig.FilenameOverride = FileName;
    if (Width > 0 && Height > 0)
        HighResConfig.SetResolution(Width, Height, 1.0f);
    else
        HighResConfig.SetResolution(GSystemResolution.ResX, GSystemResolution.ResY, 1.0f);

//     // Hook screenshot captured event
//     GEngine->GameViewport->OnScreenshotCaptured().AddLambda(
//         [=](int32, int32, const TArray<FColor>&)
//         {
//             if (GWorld)
//             {
//                 double StartTime = FPlatformTime::Seconds();
//                 FTimerHandle PollHandle;
//
//                 GWorld->GetTimerManager().SetTimer(
//                     PollHandle,
//                     [=]() mutable
//                     {
//                         double Elapsed = FPlatformTime::Seconds() - StartTime;
//
//                         if (FPaths::FileExists(TempScreenshotPath))
//                         {
// #if PLATFORM_ANDROID
//                             FString PublicPath = FString(TEXT("/storage/emulated/0/Pictures/")) + SubFolder + TEXT("/") + FileName;
//                             IFileManager::Get().MakeDirectory(*FPaths::GetPath(PublicPath), true);
//                             IFileManager::Get().Move(*PublicPath, *TempScreenshotPath);
//
//                             // Refresh Android gallery
//                             JNIEnv* Env = FAndroidApplication::GetJavaEnv();
//                             jstring PathJava = Env->NewStringUTF(TCHAR_TO_UTF8(*PublicPath));
//                             FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis,
//                                                          FJavaWrapper::AndroidThunkJava_RefreshGallery, PathJava);
//                             Env->DeleteLocalRef(PathJava);
//
// #elif PLATFORM_WINDOWS
//                             FString PublicPath = FPaths::ProjectSavedDir() / TEXT("Screenshots/") / SubFolder / FileName;
//                             IFileManager::Get().MakeDirectory(*FPaths::GetPath(PublicPath), true);
//                             IFileManager::Get().Move(*PublicPath, *TempScreenshotPath);
// #endif
//                             GWorld->GetTimerManager().ClearTimer(PollHandle);
//                         }
//                         else if (Elapsed >= TimeoutSeconds)
//                         {
//                             UE_LOG(LogTemp, Warning, TEXT("Screenshot save timeout: %s"), *TempScreenshotPath);
//                             GWorld->GetTimerManager().ClearTimer(PollHandle);
//                         }
//                     },
//                     0.2f, // check every 0.2 sec
//                     true
//                 );
//             }
//         }
    // );

    // Request screenshot
    // FScreenshotRequest::RequestScreenshot(FileName, false, false);
    bool bWasTaken = GEngine->GameViewport->GetGameViewport()->TakeHighResScreenShot();
    UE_LOG(LogTemp, Warning, TEXT("Screenshot save: %s"), bWasTaken ? TEXT("true") : TEXT("false"));
}
