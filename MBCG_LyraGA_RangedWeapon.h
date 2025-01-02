// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright DevRespawn.com (MBCG). All Rights Reserved.

// THIS CODE CAN'T BE COMPILED AS IS BECAUSE SOME PARTS WERE REMOVED DUE TO EPIC'S LICENSING TERMS.
// THIS FILE ORIGINALLY IS A MODIFICATION OF ULyraGameplayAbility_RangedWeapon CLASS FROM EPIC'S LYRA STARTER GAME.

#pragma once

// #include FILES ARE SKIPPED

#include "MBCG_LyraGA_RangedWeapon.generated.h"

// FORWARD DECALRATIONS ARE SKIPPED

/**
 * ULyraGameplayAbility_RangedWeapon
 *
 * An ability granted by and associated with a ranged weapon instance
 */
UCLASS()
class UMBCG_LyraGA_RangedWeapon : public ULyraGameplayAbility_FromEquipment
{
    GENERATED_BODY()

public:

    UMBCG_LyraGA_RangedWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    // PART OF THIS CLASS DECLARATION IS SKIPPED

    // @MBCG - START OF CODE
public:

    // Performs CameraTowardsFocusLineTrace and TargetingsSourceTowardsFocusLineTrace. Returns "false" if both hits are approximately the same, and "true" if they have different locations (which means
    // that the hit linetrace from weapon git an obstacle)
    UFUNCTION(BlueprintCallable, Category = "Lyra|Ability|MBCG")
    bool IsFireLineObstructed(FVector& OutHitLocationFromTargetingSource) const;

    UFUNCTION(BlueprintCallable, Category = "Lyra|Ability|MBCG")
    EMBCG_LyraAbilityTargetingSource GetCurrentTargetingSource() const { return CurrentTargetingSource; }
    UFUNCTION(BlueprintCallable, Category = "Lyra|Ability|MBCG")
    void SetCurrentTargetingSource(const EMBCG_LyraAbilityTargetingSource NewCurrentTargetingSource) { CurrentTargetingSource = NewCurrentTargetingSource; }

protected:

    // performs linetrace with CameraTowardsFocus and returns if there was a hit
    bool CameraTowardsFocusLineTrace(/*out*/ FHitResult& OutHitResult, /*out*/ FVector& OutStartTrace, /*out*/ FVector& OutEndTrace) const;

    // Performs linetrace with WeaponTowardsFocus or another option which is defined in runtime. Returns true if there was a hit, and false if there was no hit or something went wrong
    bool TargetingSourceTowardsFocusLineTrace(/*out*/ FHitResult& OutHitResult, /*out*/ FVector& OutStartTrace, /*out*/ FVector& OutEndTrace) const;

    // Get the hit location with traceline as if it was made with CameraTowardsFocus parameter, or calculate the virtual hit location on max trace distance
    FVector GetCameraTowardsFocusHitLocation() const;

    // Get the hit location with traceline as if it was made with WeaponTowardsFocus parameter, or calculate the virtual hit location on max trace distance
    FVector GetTargetingSourceTowardsFocusHitLocation() const;

    // Returns location of the equiped weapon. If it fails, returns error with reason
    FVector GetEquipedWeaponLocation(int& OutErrorCode) const;

    // Returns location between pawn center location (calculated by Lyra's code) and eyes location (EyesLocation), closer to eyes
    // .. used for getting bullets start trace location
    FVector GetFakeWeaponLocation(const FTransform& TargetTransform, const APawn* AvatarPawn) const;

    // Returns the location inside AvatarPawn in front of the obstacle on the line connecting the weapon and the AvatarPawn
    // .. if there is no obstacle under fire then returns WeaponLocation
    // .. used for getting location for Start trace of bullets if weapon is behind an obstacle (e.g. penetrates a wall)
    FVector GetLocationBeforePossibleObstacleOnWeaponLine(const APawn* AvatarPawn, const FVector& WeaponLocation) const;

    // performs linetrace with WeaponTowardsFocus by default (and CameraTowardsFocus during ADS) and returns if OutInputData was sucessfully filled in
    bool GetInputDataForPerformLocalTargeting(/*out*/ FRangedWeaponFiringInput& OutInputData) const;

private:

    // this variable is supposed to be set in BPs and used in this C++ class to linetrace from the targeting source defined in this variable
    // e.g. a default value can be WeaponTowardsFocus, and during ADS it can be CameraTowardsFocus
    EMBCG_LyraAbilityTargetingSource CurrentTargetingSource;

    // @MBCG - END OF CODE
};