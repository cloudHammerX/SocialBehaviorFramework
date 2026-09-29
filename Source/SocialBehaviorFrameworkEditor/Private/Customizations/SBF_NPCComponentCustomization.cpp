// Copyright Social Behavior Framework. All Rights Reserved.

#include "Customizations/SBF_NPCComponentCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "SBF_BehaviorMap.h"
#include "SBF_CharacterTrait.h"
#include "SBF_NPCComponent.h"
#include "SBF_SocialGraph.h"
#include "Widgets/Input/SObjectPropertyEntryBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FSBF_NPCComponentCustomization"

TSharedRef<IDetailCustomization> FSBF_NPCComponentCustomization::MakeInstance()
{
	return MakeShareable(new FSBF_NPCComponentCustomization());
}

void FSBF_NPCComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() == 0)
	{
		return;
	}

	USBF_NPCComponent* Component = Cast<USBF_NPCComponent>(Objects[0].Get());
	if (!Component)
	{
		return;
	}

	const TWeakObjectPtr<USBF_NPCComponent> WeakComponent = Component;

	IDetailCategoryBuilder& AssetsCategory = DetailBuilder.EditCategory(TEXT("SBF|Assets"), LOCTEXT("AssetsCategory", "SBF Assets"), ECategoryPriority::Important);

	AssetsCategory.AddCustomRow(LOCTEXT("SocialGraphRow", "Social Graph"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SocialGraphName", "Social Graph"))
			.Font(DetailBuilder.GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SObjectPropertyEntryBox)
			.AllowedClass(USBF_SocialGraph::StaticClass())
			.AllowClear(true)
			.ObjectPath_Lambda([WeakComponent]()
			{
				return (WeakComponent.IsValid() && WeakComponent->SocialGraph) ? WeakComponent->SocialGraph->GetPathName() : FString();
			})
			.OnObjectChanged_Lambda([WeakComponent](const FAssetData& AssetData)
			{
				if (WeakComponent.IsValid())
				{
					WeakComponent->Modify();
					WeakComponent->SocialGraph = Cast<USBF_SocialGraph>(AssetData.GetAsset());
					WeakComponent->MarkPackageDirty();
				}
			})
		];

	AssetsCategory.AddCustomRow(LOCTEXT("BehaviorMapRow", "Behavior Map"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BehaviorMapName", "Behavior Map"))
			.Font(DetailBuilder.GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SObjectPropertyEntryBox)
			.AllowedClass(USBF_BehaviorMap::StaticClass())
			.AllowClear(true)
			.ObjectPath_Lambda([WeakComponent]()
			{
				return (WeakComponent.IsValid() && WeakComponent->BehaviorMap) ? WeakComponent->BehaviorMap->GetPathName() : FString();
			})
			.OnObjectChanged_Lambda([WeakComponent](const FAssetData& AssetData)
			{
				if (WeakComponent.IsValid())
				{
					WeakComponent->Modify();
					WeakComponent->BehaviorMap = Cast<USBF_BehaviorMap>(AssetData.GetAsset());
					WeakComponent->MarkPackageDirty();
				}
			})
		];

	AssetsCategory.AddCustomRow(LOCTEXT("CharacterTraitRow", "Character Trait"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CharacterTraitName", "Character Trait"))
			.Font(DetailBuilder.GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SObjectPropertyEntryBox)
			.AllowedClass(USBF_CharacterTrait::StaticClass())
			.AllowClear(true)
			.ObjectPath_Lambda([WeakComponent]()
			{
				return (WeakComponent.IsValid() && WeakComponent->CharacterTrait) ? WeakComponent->CharacterTrait->GetPathName() : FString();
			})
			.OnObjectChanged_Lambda([WeakComponent](const FAssetData& AssetData)
			{
				if (WeakComponent.IsValid())
				{
					WeakComponent->Modify();
					WeakComponent->CharacterTrait = Cast<USBF_CharacterTrait>(AssetData.GetAsset());
					WeakComponent->MarkPackageDirty();
				}
			})
		];
}

#undef LOCTEXT_NAMESPACE
