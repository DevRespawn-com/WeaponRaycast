// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright DevRespawn.com (MBCG). All Rights Reserved.

// THIS CODE CAN'T BE COMPILED AS IS BECAUSE SOME PARTS WERE REMOVED DUE TO EPIC'S LICENSING TERMS.
// THIS FILE ORIGINALLY IS A MODIFICATION OF ULyraGameplayAbility_RangedWeapon CLASS FROM EPIC'S LYRA STARTER GAME.

#include "MBCG/Weapons/MBCG_LyraGA_RangedWeapon.h"
// other #include-s are skipped due to Epic's licensing terms

#include UE_INLINE_GENERATED_CPP_BY_NAME(MBCG_LyraGA_RangedWeapon)

DEFINE_LOG_CATEGORY_STATIC(LOG_UMBCG_LyraGA_RangedWeapon, All, All)

// SOME CODE IS SKIPPED

bool UMBCG_LyraGA_RangedWeapon::CameraTowardsFocusLineTrace(/*out*/ FHitResult& OutHitResult, /*out*/ FVector& OutStartTrace, /*out*/ FVector& OutEndTrace) const
{
    APawn* const AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
    check(AvatarPawn);

    // getting StartTrace and EndTrace
    const FTransform TargetTransform = GetTargetingTransform(AvatarPawn, EMBCG_LyraAbilityTargetingSource::CameraTowardsFocus);
    OutStartTrace = TargetTransform.GetTranslation();
    const FVector AimDir = TargetTransform.GetUnitAxis(EAxis::X);
    const double MaxTraceDistaance = 100000;  // cm
    OutEndTrace = OutStartTrace + AimDir * MaxTraceDistaance;

    FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(CameraTowardsFocusLineTrace), /*bTraceComplex=*/false, /*IgnoreActor=*/AvatarPawn);
    TraceParams.bReturnPhysicalMaterial = true;
    AddAdditionalTraceIgnoreActors(TraceParams);
    const ECollisionChannel TraceChannel = DetermineTraceChannel(TraceParams, /* bIsSimulated */ false);
    bool bHit = GetWorld()->LineTraceSingleByChannel(OutHitResult, OutStartTrace, OutEndTrace, TraceChannel, TraceParams);

    return bHit;
}


FVector UMBCG_LyraGA_RangedWeapon::GetCameraTowardsFocusHitLocation() const
{
    FHitResult HitResult;
    FVector StartTrace;
    FVector EndTrace;
    bool bHit = CameraTowardsFocusLineTrace(HitResult, StartTrace, EndTrace);

#if ENABLE_DRAW_DEBUG
    if (MBCG::DrawBulletTracesDuration > 0.0f)
    {
        static float DebugThickness = 1.0f;
        DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Silver, false, MBCG::DrawBulletTracesDuration, 0, DebugThickness);
    }
#endif  // ENABLE_DRAW_DEBUG

    if (bHit && HitResult.GetActor())
    {
        return HitResult.ImpactPoint;
    }
    else
    {
        // return location of max trace distance end point
        return EndTrace;
    }
}


FVector UMBCG_LyraGA_RangedWeapon::GetTargetingSourceTowardsFocusHitLocation() const
{
    FHitResult HitResult;
    FVector StartTrace;
    FVector EndTrace;
    bool bHit = TargetingSourceTowardsFocusLineTrace(HitResult, StartTrace, EndTrace);

    if (bHit && HitResult.GetActor())
    {
        return HitResult.ImpactPoint;
    }
    else
    {
        // return location of max trace distance end point
        return EndTrace;
    }
}


/**
 * @MBCG
 * Retrieves the location of the equipped weapon
 *
 * @param OutErrorCode The error code that will be set by the function.
 * 0 - no errors
 * 1 - LyraRangedWeaponInstance not found
 * 2 - Spawned weapons not found
 * 3 - Weapon actor not found
 * @return The location of the equipped weapon as an FVector.
 */
FVector UMBCG_LyraGA_RangedWeapon::GetEquipedWeaponLocation(int& OutErrorCode) const
{
    FVector Result;
    // default value
    OutErrorCode = 0;

    ULyraRangedWeaponInstance* WeaponData = GetWeaponInstance();
    if (!WeaponData)
    {
        UE_LOG(LOG_UMBCG_LyraGA_RangedWeapon, Warning, TEXT("GetEquipedWeaponLocation ErrorCode: 1"));
        OutErrorCode = 1;
        return Result;
    }

    TArray<AActor*> SpawnedWeapons = WeaponData->GetSpawnedActors();
    if (SpawnedWeapons.Num() == 0)
    {
        UE_LOG(LOG_UMBCG_LyraGA_RangedWeapon, Warning, TEXT("GetEquipedWeaponLocation ErrorCode: 2"));
        OutErrorCode = 2;
        return Result;
    }

    AActor* Weapon = SpawnedWeapons[0];
    if (!Weapon)
    {
        UE_LOG(LOG_UMBCG_LyraGA_RangedWeapon, Warning, TEXT("GetEquipedWeaponLocation ErrorCode: 3"));
        OutErrorCode = 3;
        return Result;
    }

    Result = Weapon->GetActorLocation();
    return Result;
}


FVector UMBCG_LyraGA_RangedWeapon::GetFakeWeaponLocation(const FTransform& TargetTransform, const APawn* AvatarPawn) const
{
    FVector EyesLocation;
    FRotator EyesRotation;
    AvatarPawn->GetActorEyesViewPoint(EyesLocation, EyesRotation);
    // Lyra's way: returns AvatarPawn's center
    FVector LyraStartTrace = TargetTransform.GetTranslation();
    FVector AimDirTemp = TargetTransform.GetUnitAxis(EAxis::X);
    // Fake weapon position: between pawn center location (LyraStartTrace) and eyes location (EyesLocation), closer to eyes
    // .. its drawback: when Hero moves in crouching, this calculation is wrong because the real weapon is located noticably higher
    FVector FakeWeaponLocation = (LyraStartTrace + 3 * EyesLocation) / 4;
    // WeaponOffsetFromBody is a bit less than AvatarPawn's collision capluse radius.
    // .. It is not ideal because weapons length and AvatarPawns are different, just to improve the FakeWeaponLocation
    const float WeaponOffsetFromPawn = 34.0f;
    FakeWeaponLocation = FakeWeaponLocation + AimDirTemp * WeaponOffsetFromPawn;

    return FakeWeaponLocation;
}


FVector UMBCG_LyraGA_RangedWeapon::GetLocationBeforePossibleObstacleOnWeaponLine(const APawn* AvatarPawn, const FVector& WeaponLocation) const
{
    FVector AvatarPawnLocation = AvatarPawn->GetActorLocation();
    FVector EyesLocationProjectionInsideAvatar = AvatarPawnLocation + FVector(0, 0, AvatarPawn->BaseEyeHeight);
    FVector LocationInsideAvatarOnWeaponHeight = FMath::ClosestPointOnLine(AvatarPawnLocation, EyesLocationProjectionInsideAvatar, WeaponLocation);

    // getting StartTrace and EndTrace
    const FVector StartTrace = LocationInsideAvatarOnWeaponHeight;
    const FVector EndTrace = WeaponLocation;

    FHitResult HitResult;
    FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(GetLocationBeforeObstacleOnWeaponLine), /*bTraceComplex=*/false, /*IgnoreActor=*/AvatarPawn);
    TraceParams.bReturnPhysicalMaterial = true;
    AddAdditionalTraceIgnoreActors(TraceParams);
    const ECollisionChannel TraceChannel = DetermineTraceChannel(TraceParams, /* bIsSimulated */ false);
    bool bHitObstacle = GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, TraceChannel, TraceParams);

#if ENABLE_DRAW_DEBUG
    if (MBCG::DrawBulletTracesDuration > 0.0f)
    {
        static float DebugThickness = 1.0f;
        DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Blue, false, MBCG::DrawBulletTracesDuration, 0, DebugThickness);
        DrawDebugSphere(GetWorld(), StartTrace, /*radus*/ 5.0f, /*segments*/ 12, FColor::Blue, false, MBCG::DrawBulletTracesDuration, 0, DebugThickness);
    }
#endif  // ENABLE_DRAW_DEBUG

    if (bHitObstacle && HitResult.GetActor())
    {
        return StartTrace;
    }
    else
    {
        return WeaponLocation;
    }
}


bool UMBCG_LyraGA_RangedWeapon::GetInputDataForPerformLocalTargeting(/*out*/ FRangedWeaponFiringInput& OutInputData) const
{
    // the functionality of this function was partially moved from PerformLocalTargeting() so that this function can be called independently

    // SOME CODE IS SKIPPED

    // @MBCG: Lyra's default value was CameraTowardsFocus
    // const EMBCG_LyraAbilityTargetingSource TargetingSource = EMBCG_LyraAbilityTargetingSource::WeaponTowardsFocus;
    // CurrentTargetingSource is defined in BPs: e.g. by default it is WeaponTowardsFocus, during ADS it is CameraTowardsFocus
    const EMBCG_LyraAbilityTargetingSource TargetingSource = GetCurrentTargetingSource();

    // Lyra's way to calculate TargetTransform for WeaponTowardsFocus is not good because it uses AvatarPawn's location and FocalDistance
    //... whereas we need weapon location and the distance to the aim object (if the aim is perfect)
    //... we leave TargetTransform the Lyra's way by default, but will try to specifically calculate OutInputData.StartTrace, OutInputData.EndAim and OutInputData.AimDir
    const FTransform TargetTransform = GetTargetingTransform(AvatarPawn, TargetingSource);

    // @MBCG: START OF CODE
    if ((TargetingSource == EMBCG_LyraAbilityTargetingSource::PawnTowardsFocus) || (TargetingSource == EMBCG_LyraAbilityTargetingSource::WeaponTowardsFocus))
    {
        if (TargetingSource == EMBCG_LyraAbilityTargetingSource::PawnTowardsFocus)
        {
            // This is Lyra's code. It returns AvatarPawn's location (AvatarPawn's center) in case of PawnTowardsFocus
            OutInputData.StartTrace = TargetTransform.GetTranslation();
        }

        // WeaponTowardsFocus

        // @TODO: perhaps using the weapon's muzzle location, not the weapon location
        // .. It can make sense is weapon could shoot to the direction which is different from the AvatarPawn's rotation
        // .. (e.g. in crowling, blind shooting above the haed standing backwards to the enemy etc)

        int WeaponErrorCode;
        FVector WeaponLocation;
        WeaponLocation = GetEquipedWeaponLocation(/*out*/ WeaponErrorCode);
        if (WeaponErrorCode == 0)
        {
            OutInputData.StartTrace = GetLocationBeforePossibleObstacleOnWeaponLine(AvatarPawn, WeaponLocation);
        }
        else
        {
            OutInputData.StartTrace = GetFakeWeaponLocation(TargetTransform, AvatarPawn);
        }

        // Getting the location of a hit which directs from the camera (crosshair) forward
        OutInputData.EndAim = GetCameraTowardsFocusHitLocation();

        OutInputData.AimDir = (OutInputData.EndAim - OutInputData.StartTrace).GetSafeNormal();
    }
    else
    {
        // Default Lyra's way

        // SOME CODE IS SKIPPED
    }
    // @MBCG: END OF CODE

    return true;
}


bool UMBCG_LyraGA_RangedWeapon::TargetingSourceTowardsFocusLineTrace(/*out*/ FHitResult& OutHitResult, /*out*/ FVector& OutStartTrace, /*out*/ FVector& OutEndTrace) const
{
    APawn* const AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
    FRangedWeaponFiringInput InputData;
    if (!GetInputDataForPerformLocalTargeting(InputData)) return false;
    OutStartTrace = InputData.StartTrace;
    OutEndTrace = InputData.EndAim;

    FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(TargetingSourceTowardsFocusLineTrace), /*bTraceComplex=*/false, /*IgnoreActor=*/AvatarPawn);
    TraceParams.bReturnPhysicalMaterial = true;
    AddAdditionalTraceIgnoreActors(TraceParams);
    const ECollisionChannel TraceChannel = DetermineTraceChannel(TraceParams, /* bIsSimulated */ false);
    bool bHit = GetWorld()->LineTraceSingleByChannel(OutHitResult, OutStartTrace, OutEndTrace, TraceChannel, TraceParams);

    return bHit;
}


bool UMBCG_LyraGA_RangedWeapon::IsFireLineObstructed(FVector& OutHitLocationFromTargetingSource) const
{
    const FVector HitLocationFromCamera = GetCameraTowardsFocusHitLocation();
    const FVector HitLocationFromTargetingSource = GetTargetingSourceTowardsFocusHitLocation();
    OutHitLocationFromTargetingSource = HitLocationFromTargetingSource;
#if ENABLE_DRAW_DEBUG
    if (MBCG::DrawBulletTracesDuration > 0.0f)
    {
        static float DebugThickness = 1.0f;
        DrawDebugSphere(GetWorld(), HitLocationFromCamera, /*radus*/ 7.0f, /*segments*/ 6, FColor::Yellow, false, MBCG::DrawBulletHitDuration, 0, DebugThickness);
        DrawDebugSphere(GetWorld(), HitLocationFromTargetingSource, /*radus*/ 5.0f, /*segments*/ 12, FColor::Emerald, false, MBCG::DrawBulletHitDuration, 0, DebugThickness);
    }
#endif

    // squared radius of the sphere within which the locations of the hits are considred the same
    const double DeltaSquaredRadius = 100;
    if (FVector::DistSquared(HitLocationFromCamera, HitLocationFromTargetingSource) > DeltaSquaredRadius) return true;

    return false;
}