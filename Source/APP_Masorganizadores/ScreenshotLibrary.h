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
	static void TakeCustomScreenshot(const FString& BaseFileName, const FString& SubFolder, int32 Width, int32 Height);
};

void UScreenshotLibrary::TakeCustomScreenshot(const FString& BaseFileName, const FString& SubFolder, int32 Width, int32 Height)
{
	if (!GEngine || !GEngine->GameViewport) return;

	// Filename with timestamp
	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y_%m_%d-%H_%M_%S"));
	FString FileName = FString::Printf(TEXT("%s_%s.png"), *BaseFileName, *Timestamp);

	// Get the actual default screenshot directory for the current platform/mode
	FString ScreenshotDir = FPaths::ConvertRelativePathToFull(FPaths::ScreenShotDir());
	IFileManager::Get().MakeDirectory(*ScreenshotDir, true);

	FString TempScreenshotPath = ScreenshotDir / FileName;

	// Set resolution
	FHighResScreenshotConfig& HighResConfig = GetHighResScreenshotConfig();
	HighResConfig.FilenameOverride = FileName;
	if (Width > 0 && Height > 0)
		HighResConfig.SetResolution(Width, Height, 1.0f);
	else
		HighResConfig.SetResolution(GSystemResolution.ResX, GSystemResolution.ResY, 1.0f);

	// Request screenshot
	GEngine->GameViewport->GetGameViewport()->TakeHighResScreenShot();

	FTimerHandle TimerHandle;
	GWorld->GetTimerManager().SetTimer(
		TimerHandle,
		[=]() mutable
		{
			if (FPaths::FileExists(TempScreenshotPath))
			{
#if PLATFORM_ANDROID
				FString PublicPath = FString(TEXT("/storage/emulated/0/DCIM")) / SubFolder / FileName;
				IFileManager::Get().MakeDirectory(*FPaths::GetPath(PublicPath), true);
				if (IFileManager::Get().Copy(*PublicPath, *TempScreenshotPath) == COPY_OK)
				{
					IFileManager::Get().Delete(*TempScreenshotPath); // remove from private UE folder
					UE_LOG(LogTemp, Warning, TEXT("Screenshot copied to: %s"), *PublicPath);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to copy screenshot to: %s"), *PublicPath);
				}
						
				// // Refresh Android gallery
				// JNIEnv* Env = FAndroidApplication::GetJavaEnv();
				// jstring PathJava = Env->NewStringUTF(TCHAR_TO_UTF8(*PublicPath));
				// FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis,
				// 							 FJavaWrapper::AndroidThunkJava_RefreshGallery, PathJava);
				// Env->DeleteLocalRef(PathJava);

#elif PLATFORM_WINDOWS
				FString PublicPath = FPaths::ProjectSavedDir() / TEXT("Screenshots/") / FileName;
				IFileManager::Get().MakeDirectory(*FPaths::GetPath(PublicPath), true);
				IFileManager::Get().Move(*PublicPath, *TempScreenshotPath);
				IFileManager::Get().DeleteDirectory(*ScreenshotDir);
#endif

				UE_LOG(LogTemp, Warning, TEXT("Screenshot moved to: %s"), *PublicPath);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Screenshot not found: %s"), *TempScreenshotPath);
			}
		},
		5,
		false,
		5.0f
	);
}
