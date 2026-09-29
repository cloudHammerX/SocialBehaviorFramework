// Copyright Social Behavior Framework. All Rights Reserved.

#include "Modules/ModuleManager.h"

/**
 * Runtime module of the Social Behavior Framework.
 * All runtime logic lives in the classes of the three subsystems
 * (Social Graph / Behavior Map / Character Trait); the module itself only
 * exists as the loadable unit registered by the .uplugin descriptor.
 */
class FSocialBehaviorFrameworkModule : public IModuleInterface
{
public:
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FSocialBehaviorFrameworkModule, SocialBehaviorFramework)
