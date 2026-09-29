// Copyright Social Behavior Framework. All Rights Reserved.

#include "SBF_DeveloperSettings.h"

USBF_DeveloperSettings::USBF_DeveloperSettings()
	: Super()
{
}

const USBF_DeveloperSettings& USBF_DeveloperSettings::Get()
{
	return *GetDefault<USBF_DeveloperSettings>();
}
