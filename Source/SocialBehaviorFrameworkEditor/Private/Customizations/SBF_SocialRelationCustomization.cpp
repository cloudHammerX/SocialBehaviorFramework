// Copyright Social Behavior Framework. All Rights Reserved.

#include "Customizations/SBF_SocialRelationCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "SBF_SocialRelation.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FSBF_SocialRelationCustomization"

TSharedRef<IDetailCustomization> FSBF_SocialRelationCustomization::MakeInstance()
{
	return MakeShareable(new FSBF_SocialRelationCustomization());
}

void FSBF_SocialRelationCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() == 0)
	{
		return;
	}

	USBF_SocialRelation* Relation = Cast<USBF_SocialRelation>(Objects[0].Get());
	if (!Relation)
	{
		return;
	}

	// Weak capture: the customization instance may outlive the selection.
	const TWeakObjectPtr<USBF_SocialRelation> WeakRelation = Relation;

	IDetailCategoryBuilder& StanceCategory = DetailBuilder.EditCategory(TEXT("Relation"), LOCTEXT("RelationCategory", "Relation"), ECategoryPriority::Important);
	StanceCategory.AddCustomRow(LOCTEXT("DefaultStanceRow", "Default Stance"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("DefaultStance", "Default Stance"))
			.Font(DetailBuilder.GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SBox)
			.Padding(FMargin(0.0f, 2.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("SetAllyStance", "Ally"))
					.ToolTipText(LOCTEXT("SetAllyStanceTooltip", "Adds the relation tag to AllyTags (self-inclusion = allied relation)."))
					.OnClicked_Lambda([WeakRelation]()
					{
						if (USBF_SocialRelation* Rel = WeakRelation.Get())
						{
							Rel->Modify();
							Rel->AllyTags.AddTag(Rel->RelationTag);
							Rel->EnemyTags.RemoveTag(Rel->RelationTag);
							Rel->MarkPackageDirty();
						}
						return FReply::Handled();
					})
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("SetEnemyStance", "Enemy"))
					.ToolTipText(LOCTEXT("SetEnemyStanceTooltip", "Adds the relation tag to EnemyTags (self-inclusion = hostile relation)."))
					.OnClicked_Lambda([WeakRelation]()
					{
						if (USBF_SocialRelation* Rel = WeakRelation.Get())
						{
							Rel->Modify();
							Rel->EnemyTags.AddTag(Rel->RelationTag);
							Rel->AllyTags.RemoveTag(Rel->RelationTag);
							Rel->MarkPackageDirty();
						}
						return FReply::Handled();
					})
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("SetNeutralStance", "Neutral"))
					.ToolTipText(LOCTEXT("SetNeutralStanceTooltip", "Removes the relation tag from both containers (neutral relation)."))
					.OnClicked_Lambda([WeakRelation]()
					{
						if (USBF_SocialRelation* Rel = WeakRelation.Get())
						{
							Rel->Modify();
							Rel->AllyTags.RemoveTag(Rel->RelationTag);
							Rel->EnemyTags.RemoveTag(Rel->RelationTag);
							Rel->MarkPackageDirty();
						}
						return FReply::Handled();
					})
				]
			]
		];
}

#undef LOCTEXT_NAMESPACE
