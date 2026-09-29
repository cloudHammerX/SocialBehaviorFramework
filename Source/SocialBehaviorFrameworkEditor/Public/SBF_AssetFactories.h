// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "SBF_AssetFactories.generated.h"

/**
 * UFactory classes used to create SBF data assets from the Content Browser
 * and from the editor window's "New ..." buttons (via FAssetToolsModule /
 * UDataAssetFactory-style creation).
 */

/** Creates USBF_SocialGraph assets with a set of default relation assets. */
UCLASS()
class SOCIALBEHAVIORFRAMEWORKEDITOR_API USBF_SocialGraphFactory : public UFactory
{
	GENERATED_BODY()

public:
	USBF_SocialGraphFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override { return true; }
};

/** Creates USBF_BehaviorMap assets with a default 9-18 work day. */
UCLASS()
class SOCIALBEHAVIORFRAMEWORKEDITOR_API USBF_BehaviorMapFactory : public UFactory
{
	GENERATED_BODY()

public:
	USBF_BehaviorMapFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override { return true; }
};

/** Creates USBF_CharacterTrait assets with default Threat/Offender reactions. */
UCLASS()
class SOCIALBEHAVIORFRAMEWORKEDITOR_API USBF_CharacterTraitFactory : public UFactory
{
	GENERATED_BODY()

public:
	USBF_CharacterTraitFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override { return true; }
};

/** Creates USBF_TagLibrary assets pre-filled with the default trigger tags. */
UCLASS()
class SOCIALBEHAVIORFRAMEWORKEDITOR_API USBF_TagLibraryFactory : public UFactory
{
	GENERATED_BODY()

public:
	USBF_TagLibraryFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override { return true; }
};

/** Creates USBF_SocialRelation assets. */
UCLASS()
class SOCIALBEHAVIORFRAMEWORKEDITOR_API USBF_SocialRelationFactory : public UFactory
{
	GENERATED_BODY()

public:
	USBF_SocialRelationFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override { return true; }
};
