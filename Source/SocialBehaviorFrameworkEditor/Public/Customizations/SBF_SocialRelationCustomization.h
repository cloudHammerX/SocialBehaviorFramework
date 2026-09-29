// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;

/**
 * Details customization for USBF_SocialRelation.
 *
 * Adds quick "default stance" buttons (Ally / Enemy / Neutral) that update
 * the relation's AllyTags / EnemyTags using the self-inclusion convention
 * (own tag included = the relation itself is allied / hostile).
 */
class SOCIALBEHAVIORFRAMEWORKEDITOR_API FSBF_SocialRelationCustomization : public IDetailCustomization
{
public:
	/** @return a new customization instance (FOnGetDetailCustomizationInstance). */
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
