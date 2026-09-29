// Copyright Social Behavior Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;

/**
 * Details customization for USBF_NPCComponent: renders the three SBF asset
 * references (Social Graph / Behavior Map / Character Trait) as asset picker
 * rows at the top of the details panel (attachment customization).
 */
class SOCIALBEHAVIORFRAMEWORKEDITOR_API FSBF_NPCComponentCustomization : public IDetailCustomization
{
public:
	/** @return a new customization instance (FOnGetDetailCustomizationInstance). */
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
