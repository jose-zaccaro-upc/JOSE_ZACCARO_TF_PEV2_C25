// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "UI/PEV2_GameHUDWidget.h"
#include "Gameplay/Weapons/PEV2_WeaponBase.h"
#include "Gameplay/Items/PEV2_AbilityData.h"
#include "Characters/PEV2_Character.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Images/SImage.h"
#include "Styling/CoreStyle.h"
#include "Engine/Texture2D.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Blueprint/WidgetTree.h"
#include "Rendering/Texture2DResource.h"

TMap<FString, TObjectPtr<UTexture2D>> UPEV2_GameHUDWidget::StaticWeaponIcons;
TMap<FString, TObjectPtr<UTexture2D>> UPEV2_GameHUDWidget::StaticAbilityIcons;

/**
 * Dedicated Slate compound widget providing high-performance,
 * zero-asset HUD rendering for Gun Master Arena.
 */
class SPEV2_GameHUD : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPEV2_GameHUD) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		const FSlateBrush* DefaultBrush = FCoreStyle::Get().GetDefaultBrush();

		FSlateFontInfo TitleFont = FCoreStyle::Get().GetFontStyle("NormalFont");
		TitleFont.Size = 12;

		FSlateFontInfo ValueFont = FCoreStyle::Get().GetFontStyle("NormalFont");
		ValueFont.Size = 11;

		FSlateFontInfo SubTitleFont = FCoreStyle::Get().GetFontStyle("NormalFont");
		SubTitleFont.Size = 9;

		FSlateFontInfo KeyBadgeFont = FCoreStyle::Get().GetFontStyle("NormalFont");
		KeyBadgeFont.Size = 10;

		AbilityBrush = MakeShared<FSlateBrush>(*DefaultBrush);

		ChildSlot
		[
			SNew(SOverlay)

			// ==========================================
			// BOTTOM-LEFT (STACKED): TACTICAL ABILITY CARD ("Q")
			// ==========================================
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(FMargin(36.0f, 0.0f, 0.0f, 142.0f))
			[
				SNew(SBox)
				.WidthOverride(260.0f)
				[
					SAssignNew(AbilityCardBorder, SBorder)
					.BorderImage(DefaultBrush)
					.BorderBackgroundColor(FLinearColor(0.04f, 0.08f, 0.12f, 0.85f))
					.Padding(FMargin(10.0f, 6.0f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							// [Q] Key Badge
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(0.0f, 0.0f, 8.0f, 0.0f)
							[
								SNew(SBorder)
								.BorderImage(DefaultBrush)
								.BorderBackgroundColor(FLinearColor(0.08f, 0.35f, 0.55f, 0.9f))
								.Padding(FMargin(5.0f, 2.0f))
								[
									SNew(STextBlock)
									.Text(FText::FromString(TEXT("Q")))
									.Font(KeyBadgeFont)
									.ColorAndOpacity(FSlateColor(FLinearColor(0.3f, 0.95f, 1.0f, 1.0f)))
								]
							]
							// Ability Icon
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(0.0f, 0.0f, 8.0f, 0.0f)
							[
								SNew(SBox)
								.WidthOverride(32.0f)
								.HeightOverride(32.0f)
								[
									SAssignNew(AbilityIconImage, SImage)
									.Image(AbilityBrush.Get())
									.ColorAndOpacity(FLinearColor(0.3f, 0.95f, 1.0f, 1.0f))
								]
							]
							// Ability Info (Name & Status)
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.VAlign(VAlign_Center)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SAssignNew(AbilityNameText, STextBlock)
									.Text(FText::FromString(TEXT("HABILIDAD")))
									.Font(SubTitleFont)
									.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.95f, 1.0f, 1.0f)))
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.AutoWidth()
									[
										SAssignNew(AbilityStatusText, STextBlock)
										.Text(FText::FromString(TEXT("LISTA")))
										.Font(SubTitleFont)
										.ColorAndOpacity(FSlateColor(FLinearColor(0.2f, 1.0f, 0.5f, 1.0f)))
									]
									+ SHorizontalBox::Slot()
									.FillWidth(1.0f)
									.HAlign(HAlign_Right)
									[
										SAssignNew(AbilityChargesText, STextBlock)
										.Text(FText::FromString(TEXT("x1")))
										.Font(SubTitleFont)
										.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.9f, 1.0f, 0.85f)))
									]
								]
							]
						]
						// Cooldown Bar
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 0.0f)
						[
							SNew(SBox)
							.HeightOverride(4.0f)
							[
								SAssignNew(AbilityCooldownBar, SProgressBar)
								.Percent(1.0f)
								.FillColorAndOpacity(FLinearColor(0.15f, 0.85f, 1.0f, 1.0f))
							]
						]
					]
				]
			]

			// ==========================================
			// BOTTOM-LEFT: SURVIVAL CARD (ESCUDO & VIDA)
			// ==========================================
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(FMargin(36.0f, 0.0f, 0.0f, 36.0f))
			[
				SNew(SBox)
				.WidthOverride(260.0f)
				[
					SNew(SBorder)
					.BorderImage(DefaultBrush)
					.BorderBackgroundColor(FLinearColor(0.03f, 0.04f, 0.07f, 0.88f))
					.Padding(FMargin(14.0f, 8.0f))
					[
						SNew(SVerticalBox)
						// --- SECCIÓN ESCUDO ---
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 3.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SAssignNew(ShieldLabelText, STextBlock)
								.Text(FText::FromString(TEXT("ESCUDO")))
								.Font(TitleFont)
								.ColorAndOpacity(FSlateColor(FLinearColor(0.2f, 0.85f, 1.0f, 1.0f)))
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.HAlign(HAlign_Right)
							[
								SAssignNew(ShieldValueText, STextBlock)
								.Text(FText::FromString(TEXT("0 / 50")))
								.Font(ValueFont)
								.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.95f, 1.0f, 0.95f)))
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 8.0f)
						[
							SNew(SBox)
							.HeightOverride(8.0f)
							[
								SAssignNew(ShieldProgressBar, SProgressBar)
								.Percent(0.0f)
								.FillColorAndOpacity(FLinearColor(0.15f, 0.75f, 1.0f, 1.0f))
							]
						]
						// --- SECCIÓN VIDA ---
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 3.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SAssignNew(HealthLabelText, STextBlock)
								.Text(FText::FromString(TEXT("VIDA")))
								.Font(TitleFont)
								.ColorAndOpacity(FSlateColor(FLinearColor(0.2f, 1.0f, 0.45f, 1.0f)))
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.HAlign(HAlign_Right)
							[
								SAssignNew(HealthValueText, STextBlock)
								.Text(FText::FromString(TEXT("100 / 100")))
								.Font(ValueFont)
								.ColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.95f, 1.0f, 0.95f)))
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SBox)
							.HeightOverride(10.0f)
							[
								SAssignNew(HealthProgressBar, SProgressBar)
								.Percent(1.0f)
								.FillColorAndOpacity(FLinearColor(0.15f, 0.85f, 0.4f, 1.0f))
							]
						]
					]
				]
			]

			// ==========================================
			// BOTTOM-CENTER: ARSENAL / WEAPON ROSTER DOCK
			// ==========================================
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Bottom)
			.Padding(FMargin(0.0f, 0.0f, 0.0f, 36.0f))
			[
				SNew(SBorder)
				.BorderImage(DefaultBrush)
				.BorderBackgroundColor(FLinearColor(0.02f, 0.03f, 0.05f, 0.82f))
				.Padding(FMargin(12.0f, 8.0f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 6.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("ARSENAL - CARRERA DE ARMAS")))
						.Font(SubTitleFont)
						.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.75f, 0.95f, 0.75f)))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(RosterHorizontalBox, SHorizontalBox)
					]
				]
			]

			// ==========================================
			// BOTTOM-RIGHT: AMMO CARD ("BALAS")
			// ==========================================
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(FMargin(0.0f, 0.0f, 36.0f, 36.0f))
			[
				SNew(SBox)
				.WidthOverride(260.0f)
				[
					SNew(SBorder)
					.BorderImage(DefaultBrush)
					.BorderBackgroundColor(FLinearColor(0.03f, 0.04f, 0.07f, 0.85f))
					.Padding(FMargin(14.0f, 10.0f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 4.0f)
						[
							SAssignNew(EquippedWeaponNameText, STextBlock)
							.Text(FText::FromString(TEXT("ARMA")))
							.Font(SubTitleFont)
							.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.9f, 1.0f, 0.85f)))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 6.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SAssignNew(AmmoLabelText, STextBlock)
								.Text(FText::FromString(TEXT("BALAS")))
								.Font(TitleFont)
								.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.8f, 0.18f, 1.0f)))
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.HAlign(HAlign_Right)
							[
								SAssignNew(AmmoValueText, STextBlock)
								.Text(FText::FromString(TEXT("30 / 30")))
								.Font(TitleFont)
								.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.95f, 0.8f, 1.0f)))
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SBox)
							.HeightOverride(12.0f)
							[
								SAssignNew(AmmoProgressBar, SProgressBar)
								.Percent(1.0f)
								.FillColorAndOpacity(FLinearColor(0.18f, 0.72f, 1.0f, 1.0f))
							]
						]
					]
				]
			]
		];
	}

	void UpdateHealth(float InHealth, float InMaxHealth, float InPercent)
	{
		if (HealthProgressBar.IsValid())
		{
			HealthProgressBar->SetPercent(InPercent);
			if (InPercent > 0.5f)
			{
				HealthProgressBar->SetFillColorAndOpacity(FLinearColor(0.15f, 0.85f, 0.4f, 1.0f));
			}
			else if (InPercent > 0.25f)
			{
				HealthProgressBar->SetFillColorAndOpacity(FLinearColor(0.95f, 0.65f, 0.1f, 1.0f));
			}
			else
			{
				HealthProgressBar->SetFillColorAndOpacity(FLinearColor(0.95f, 0.15f, 0.15f, 1.0f));
			}
		}

		if (HealthValueText.IsValid())
		{
			const int32 DisplayHealth = FMath::Max(0, FMath::RoundToInt(InHealth));
			const int32 DisplayMax = FMath::Max(1, FMath::RoundToInt(InMaxHealth));
			HealthValueText->SetText(FText::Format(FText::FromString(TEXT("{0} / {1}")), FText::AsNumber(DisplayHealth), FText::AsNumber(DisplayMax)));
		}
	}

	void UpdateShield(float InShield, float InMaxShield, float InPercent)
	{
		if (ShieldProgressBar.IsValid())
		{
			ShieldProgressBar->SetPercent(InPercent);
			if (InPercent > 0.25f)
			{
				ShieldProgressBar->SetFillColorAndOpacity(FLinearColor(0.15f, 0.75f, 1.0f, 1.0f));
			}
			else
			{
				ShieldProgressBar->SetFillColorAndOpacity(FLinearColor(0.1f, 0.45f, 0.95f, 0.85f));
			}
		}

		if (ShieldValueText.IsValid())
		{
			const int32 DisplayShield = FMath::Max(0, FMath::RoundToInt(InShield));
			const int32 DisplayMax = FMath::Max(1, FMath::RoundToInt(InMaxShield));
			ShieldValueText->SetText(FText::Format(FText::FromString(TEXT("{0} / {1}")), FText::AsNumber(DisplayShield), FText::AsNumber(DisplayMax)));
		}
	}

	void UpdateAmmo(int32 InAmmo, int32 InMaxAmmo, bool bInIsMelee, const FText& InWeaponName)
	{
		if (EquippedWeaponNameText.IsValid() && !InWeaponName.IsEmpty())
		{
			EquippedWeaponNameText->SetText(InWeaponName);
		}

		if (bInIsMelee || InMaxAmmo <= 0)
		{
			if (AmmoProgressBar.IsValid())
			{
				AmmoProgressBar->SetPercent(1.0f);
				AmmoProgressBar->SetFillColorAndOpacity(FLinearColor(0.85f, 0.85f, 0.85f, 0.9f));
			}
			if (AmmoValueText.IsValid())
			{
				AmmoValueText->SetText(FText::FromString(TEXT("∞")));
			}
		}
		else
		{
			const float AmmoPercent = (InMaxAmmo > 0) ? (float)InAmmo / (float)InMaxAmmo : 0.0f;
			if (AmmoProgressBar.IsValid())
			{
				AmmoProgressBar->SetPercent(AmmoPercent);
				if (AmmoPercent > 0.3f)
				{
					AmmoProgressBar->SetFillColorAndOpacity(FLinearColor(0.18f, 0.72f, 1.0f, 1.0f));
				}
				else
				{
					AmmoProgressBar->SetFillColorAndOpacity(FLinearColor(1.0f, 0.35f, 0.1f, 1.0f));
				}
			}
			if (AmmoValueText.IsValid())
			{
				AmmoValueText->SetText(FText::Format(FText::FromString(TEXT("{0} / {1}")), FText::AsNumber(InAmmo), FText::AsNumber(InMaxAmmo)));
			}
		}
	}

	void UpdateAbility(UPEV2_AbilityData* InAbilityData, int32 InCharges, float InRemainingCooldown)
	{
		if (!AbilityNameText.IsValid() || !AbilityStatusText.IsValid())
		{
			return;
		}

		if (InAbilityData)
		{
			AbilityNameText->SetText(InAbilityData->AbilityName);

			if (InCharges > 0 && InRemainingCooldown <= 0.05f)
			{
				AbilityStatusText->SetText(FText::FromString(TEXT("LISTA")));
				AbilityStatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 1.0f, 0.5f, 1.0f)));
				if (AbilityCardBorder.IsValid())
				{
					AbilityCardBorder->SetBorderBackgroundColor(FLinearColor(0.05f, 0.18f, 0.28f, 0.88f));
				}
				if (AbilityCooldownBar.IsValid())
				{
					AbilityCooldownBar->SetPercent(1.0f);
					AbilityCooldownBar->SetFillColorAndOpacity(FLinearColor(0.15f, 0.85f, 1.0f, 1.0f));
				}
			}
			else
			{
				const int32 Secs = FMath::CeilToInt(FMath::Max(0.0f, InRemainingCooldown));
				AbilityStatusText->SetText(FText::Format(FText::FromString(TEXT("{0}s")), FText::AsNumber(Secs)));
				AbilityStatusText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.7f, 0.2f, 1.0f)));
				if (AbilityCardBorder.IsValid())
				{
					AbilityCardBorder->SetBorderBackgroundColor(FLinearColor(0.05f, 0.06f, 0.08f, 0.8f));
				}
			}

			if (AbilityChargesText.IsValid())
			{
				AbilityChargesText->SetText(FText::Format(FText::FromString(TEXT("x{0}")), FText::AsNumber(InCharges)));
				AbilityChargesText->SetVisibility(EVisibility::Visible);
			}

			if (UTexture2D* IconTex = UPEV2_GameHUDWidget::GetOrCreateIconForAbility(InAbilityData, InAbilityData->AbilityName.ToString()))
			{
				if (AbilityBrush.IsValid())
				{
					AbilityBrush->SetResourceObject(IconTex);
					AbilityBrush->ImageSize = FVector2D(32.0f, 32.0f);
					AbilityBrush->DrawAs = ESlateBrushDrawType::Image;
				}
			}
		}
		else
		{
			AbilityNameText->SetText(FText::FromString(TEXT("HABILIDAD [VACÍO]")));
			AbilityStatusText->SetText(FText::FromString(TEXT("NINGUNA")));
			AbilityStatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.6f, 0.7f, 0.5f)));

			if (AbilityChargesText.IsValid())
			{
				AbilityChargesText->SetVisibility(EVisibility::Collapsed);
			}

			if (AbilityCardBorder.IsValid())
			{
				AbilityCardBorder->SetBorderBackgroundColor(FLinearColor(0.02f, 0.03f, 0.05f, 0.65f));
			}

			if (AbilityCooldownBar.IsValid())
			{
				AbilityCooldownBar->SetPercent(0.0f);
			}

			if (UTexture2D* NoneIcon = UPEV2_GameHUDWidget::GetOrCreateIconForAbility(nullptr, TEXT("None")))
			{
				if (AbilityBrush.IsValid())
				{
					AbilityBrush->SetResourceObject(NoneIcon);
					AbilityBrush->ImageSize = FVector2D(32.0f, 32.0f);
					AbilityBrush->DrawAs = ESlateBrushDrawType::Image;
				}
			}
		}
	}

	void UpdateAbilityCooldown(float InRemainingCooldown, float InTotalDuration)
	{
		if (!AbilityCooldownBar.IsValid() || !AbilityStatusText.IsValid())
		{
			return;
		}

		if (InRemainingCooldown > 0.05f && InTotalDuration > 0.0f)
		{
			const float Ratio = FMath::Clamp(1.0f - (InRemainingCooldown / InTotalDuration), 0.0f, 1.0f);
			AbilityCooldownBar->SetPercent(Ratio);
			AbilityCooldownBar->SetFillColorAndOpacity(FLinearColor(1.0f, 0.65f, 0.15f, 1.0f));

			const int32 Secs = FMath::CeilToInt(InRemainingCooldown);
			AbilityStatusText->SetText(FText::Format(FText::FromString(TEXT("{0}s")), FText::AsNumber(Secs)));
			AbilityStatusText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.7f, 0.2f, 1.0f)));
		}
		else
		{
			AbilityCooldownBar->SetPercent(1.0f);
			AbilityCooldownBar->SetFillColorAndOpacity(FLinearColor(0.15f, 0.85f, 1.0f, 1.0f));
			AbilityStatusText->SetText(FText::FromString(TEXT("LISTA")));
			AbilityStatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 1.0f, 0.5f, 1.0f)));
		}
	}

	void SetRosterItems(const TArray<FPEV2_HUDWeaponItem>& Items, int32 InActiveTier)
	{
		if (!RosterHorizontalBox.IsValid())
		{
			return;
		}

		RosterHorizontalBox->ClearChildren();
		SlotWidgetsArray.Empty();

		const FSlateBrush* DefaultBrush = FCoreStyle::Get().GetDefaultBrush();

		FSlateFontInfo BadgeFont = FCoreStyle::Get().GetFontStyle("NormalFont");
		BadgeFont.Size = 9;

		FSlateFontInfo NameFont = FCoreStyle::Get().GetFontStyle("NormalFont");
		NameFont.Size = 8;

		const FLinearColor ActiveBorderColor(1.0f, 0.82f, 0.12f, 1.0f);
		const FLinearColor InactiveBorderColor(0.2f, 0.24f, 0.32f, 0.55f);

		const FLinearColor ActiveBgColor(0.24f, 0.18f, 0.04f, 0.92f);
		const FLinearColor InactiveBgColor(0.04f, 0.05f, 0.08f, 0.72f);

		for (int32 i = 0; i < Items.Num(); ++i)
		{
			const FPEV2_HUDWeaponItem& Item = Items[i];
			const bool bIsActive = (Item.TierIndex == InActiveTier);

			TSharedPtr<FSlateBrush> DynamicBrush = MakeShared<FSlateBrush>();
			if (Item.WeaponIcon)
			{
				DynamicBrush->SetResourceObject(Item.WeaponIcon);
				DynamicBrush->ImageSize = FVector2D(64.0f, 32.0f);
				DynamicBrush->DrawAs = ESlateBrushDrawType::Image;
			}
			else
			{
				DynamicBrush = MakeShared<FSlateBrush>(*DefaultBrush);
			}

			TSharedPtr<SBorder> CardBorder;
			TSharedPtr<STextBlock> NameText;
			TSharedPtr<STextBlock> ActiveBadgeText;
			TSharedPtr<SImage> IconImage;

			RosterHorizontalBox->AddSlot()
				.AutoWidth()
				.Padding(2.5f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(76.0f)
					.HeightOverride(54.0f)
					[
						SAssignNew(CardBorder, SBorder)
						.BorderImage(DefaultBrush)
						.BorderBackgroundColor(bIsActive ? ActiveBgColor : InactiveBgColor)
						.Padding(FMargin(4.0f, 3.0f))
						[
							SNew(SVerticalBox)
							// Top row: Tier index + "ACTIVA"
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(STextBlock)
									.Text(FText::AsNumber(Item.TierIndex + 1))
									.Font(BadgeFont)
									.ColorAndOpacity(bIsActive ? FSlateColor(ActiveBorderColor) : FSlateColor(FLinearColor(0.55f, 0.6f, 0.7f, 0.6f)))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.HAlign(HAlign_Right)
								[
									SAssignNew(ActiveBadgeText, STextBlock)
									.Text(FText::FromString(TEXT("ACTIVA")))
									.Font(BadgeFont)
									.ColorAndOpacity(FSlateColor(ActiveBorderColor))
									.Visibility(bIsActive ? EVisibility::Visible : EVisibility::Collapsed)
								]
							]
							// Center: Weapon Icon
							+ SVerticalBox::Slot()
							.FillHeight(1.0f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(46.0f)
								.HeightOverride(22.0f)
								[
									SAssignNew(IconImage, SImage)
									.Image(DynamicBrush.Get())
									.ColorAndOpacity(bIsActive ? FLinearColor::White : FLinearColor(0.7f, 0.75f, 0.8f, 0.45f))
								]
							]
							// Bottom: Weapon Name
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							[
								SAssignNew(NameText, STextBlock)
								.Text(Item.WeaponName)
								.Font(NameFont)
								.ColorAndOpacity(bIsActive ? FSlateColor(FLinearColor(1.0f, 0.95f, 0.75f, 1.0f)) : FSlateColor(FLinearColor(0.6f, 0.65f, 0.7f, 0.55f)))
							]
						]
					]
				];

			FSlotWidgets Slot;
			Slot.TierIndex = Item.TierIndex;
			Slot.CardBorder = CardBorder;
			Slot.NameText = NameText;
			Slot.ActiveBadgeText = ActiveBadgeText;
			Slot.IconImage = IconImage;
			Slot.Brush = DynamicBrush;
			SlotWidgetsArray.Add(Slot);
		}
	}

	void SetActiveTier(int32 InActiveTier)
	{
		const FLinearColor ActiveBgColor(0.24f, 0.18f, 0.04f, 0.92f);
		const FLinearColor InactiveBgColor(0.04f, 0.05f, 0.08f, 0.72f);

		for (FSlotWidgets& Slot : SlotWidgetsArray)
		{
			const bool bIsActive = (Slot.TierIndex == InActiveTier);
			if (Slot.CardBorder.IsValid())
			{
				Slot.CardBorder->SetBorderBackgroundColor(bIsActive ? ActiveBgColor : InactiveBgColor);
			}
			if (Slot.NameText.IsValid())
			{
				Slot.NameText->SetColorAndOpacity(bIsActive ? FSlateColor(FLinearColor(1.0f, 0.95f, 0.75f, 1.0f)) : FSlateColor(FLinearColor(0.6f, 0.65f, 0.7f, 0.55f)));
			}
			if (Slot.ActiveBadgeText.IsValid())
			{
				Slot.ActiveBadgeText->SetVisibility(bIsActive ? EVisibility::Visible : EVisibility::Collapsed);
			}
			if (Slot.IconImage.IsValid())
			{
				Slot.IconImage->SetColorAndOpacity(bIsActive ? FLinearColor::White : FLinearColor(0.7f, 0.75f, 0.8f, 0.45f));
			}
		}
	}

private:
	TSharedPtr<SProgressBar> ShieldProgressBar;
	TSharedPtr<STextBlock> ShieldLabelText;
	TSharedPtr<STextBlock> ShieldValueText;

	TSharedPtr<SProgressBar> HealthProgressBar;
	TSharedPtr<STextBlock> HealthLabelText;
	TSharedPtr<STextBlock> HealthValueText;

	TSharedPtr<SProgressBar> AmmoProgressBar;
	TSharedPtr<STextBlock> AmmoLabelText;
	TSharedPtr<STextBlock> AmmoValueText;
	TSharedPtr<STextBlock> EquippedWeaponNameText;

	TSharedPtr<SBorder> AbilityCardBorder;
	TSharedPtr<SImage> AbilityIconImage;
	TSharedPtr<STextBlock> AbilityNameText;
	TSharedPtr<STextBlock> AbilityStatusText;
	TSharedPtr<STextBlock> AbilityChargesText;
	TSharedPtr<SProgressBar> AbilityCooldownBar;
	TSharedPtr<FSlateBrush> AbilityBrush;

	TSharedPtr<SHorizontalBox> RosterHorizontalBox;

	struct FSlotWidgets
	{
		int32 TierIndex = 0;
		TSharedPtr<SBorder> CardBorder;
		TSharedPtr<STextBlock> NameText;
		TSharedPtr<STextBlock> ActiveBadgeText;
		TSharedPtr<SImage> IconImage;
		TSharedPtr<FSlateBrush> Brush;
	};

	TArray<FSlotWidgets> SlotWidgetsArray;
};

// ==========================================
// UPEV2_GameHUDWidget IMPLEMENTATION
// ==========================================

UPEV2_GameHUDWidget::UPEV2_GameHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPEV2_GameHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UpdateHealth(CachedHealth, CachedMaxHealth, CachedHealthPercent);
	UpdateShield(CachedShield, CachedMaxShield, CachedShieldPercent);
	UpdateAmmo(CachedAmmo, CachedMaxAmmo, bCachedIsMelee, CachedWeaponName);
	UpdateAbility(CachedAbilityData.Get(), CachedAbilityCharges, CachedRemainingCooldown);

	if (CachedRosterItems.Num() > 0)
	{
		if (SlateHUDWidget.IsValid())
		{
			SlateHUDWidget->SetRosterItems(CachedRosterItems, CachedActiveTier);
		}
	}
	else if (APEV2_Character* OwnerChar = Cast<APEV2_Character>(GetOwningPlayerPawn()))
	{
		InitializeRoster(OwnerChar->GetWeaponTierClasses(), OwnerChar->GetCurrentWeaponTier());
	}
}

TSharedRef<SWidget> UPEV2_GameHUDWidget::RebuildWidget()
{
	if (WidgetTree && WidgetTree->RootWidget)
	{
		return Super::RebuildWidget();
	}

	SAssignNew(SlateHUDWidget, SPEV2_GameHUD);

	if (SlateHUDWidget.IsValid())
	{
		SlateHUDWidget->UpdateHealth(CachedHealth, CachedMaxHealth, CachedHealthPercent);
		SlateHUDWidget->UpdateShield(CachedShield, CachedMaxShield, CachedShieldPercent);
		SlateHUDWidget->UpdateAmmo(CachedAmmo, CachedMaxAmmo, bCachedIsMelee, CachedWeaponName);
		SlateHUDWidget->UpdateAbility(CachedAbilityData.Get(), CachedAbilityCharges, CachedRemainingCooldown);
		if (CachedRosterItems.Num() > 0)
		{
			SlateHUDWidget->SetRosterItems(CachedRosterItems, CachedActiveTier);
		}
	}

	return SlateHUDWidget.ToSharedRef();
}

void UPEV2_GameHUDWidget::UpdateHealth(float InCurrentHealth, float InMaxHealth, float InLifePercent)
{
	CachedHealth = InCurrentHealth;
	CachedMaxHealth = InMaxHealth;
	CachedHealthPercent = InLifePercent;

	if (SlateHUDWidget.IsValid())
	{
		SlateHUDWidget->UpdateHealth(InCurrentHealth, InMaxHealth, InLifePercent);
	}

	if (ProgressBar_Health)
	{
		ProgressBar_Health->SetPercent(InLifePercent);
	}

	if (Text_HealthLabel)
	{
		Text_HealthLabel->SetText(FText::FromString(TEXT("VIDA")));
	}

	if (Text_HealthValue)
	{
		const int32 DisplayHealth = FMath::Max(0, FMath::RoundToInt(InCurrentHealth));
		const int32 DisplayMax = FMath::Max(1, FMath::RoundToInt(InMaxHealth));
		Text_HealthValue->SetText(FText::Format(FText::FromString(TEXT("{0} / {1}")), FText::AsNumber(DisplayHealth), FText::AsNumber(DisplayMax)));
	}
}

void UPEV2_GameHUDWidget::UpdateShield(float InCurrentShield, float InMaxShield, float InShieldPercent)
{
	CachedShield = InCurrentShield;
	CachedMaxShield = InMaxShield;
	CachedShieldPercent = InShieldPercent;

	if (SlateHUDWidget.IsValid())
	{
		SlateHUDWidget->UpdateShield(InCurrentShield, InMaxShield, InShieldPercent);
	}

	if (ProgressBar_Shield)
	{
		ProgressBar_Shield->SetPercent(InShieldPercent);
	}

	if (Text_ShieldLabel)
	{
		Text_ShieldLabel->SetText(FText::FromString(TEXT("ESCUDO")));
	}

	if (Text_ShieldValue)
	{
		const int32 DisplayShield = FMath::Max(0, FMath::RoundToInt(InCurrentShield));
		const int32 DisplayMax = FMath::Max(1, FMath::RoundToInt(InMaxShield));
		Text_ShieldValue->SetText(FText::Format(FText::FromString(TEXT("{0} / {1}")), FText::AsNumber(DisplayShield), FText::AsNumber(DisplayMax)));
	}
}

void UPEV2_GameHUDWidget::UpdateAmmo(int32 InCurrentAmmo, int32 InMaxAmmo, bool bInIsMelee, const FText& InWeaponName)
{
	CachedAmmo = InCurrentAmmo;
	CachedMaxAmmo = InMaxAmmo;
	bCachedIsMelee = bInIsMelee;
	CachedWeaponName = InWeaponName;

	if (SlateHUDWidget.IsValid())
	{
		SlateHUDWidget->UpdateAmmo(InCurrentAmmo, InMaxAmmo, bInIsMelee, InWeaponName);
	}

	if (Text_EquippedWeaponName && !InWeaponName.IsEmpty())
	{
		Text_EquippedWeaponName->SetText(InWeaponName);
	}

	if (Text_AmmoLabel)
	{
		Text_AmmoLabel->SetText(FText::FromString(TEXT("BALAS")));
	}

	if (bInIsMelee || InMaxAmmo <= 0)
	{
		if (ProgressBar_Ammo)
		{
			ProgressBar_Ammo->SetPercent(1.0f);
		}
		if (Text_AmmoValue)
		{
			Text_AmmoValue->SetText(FText::FromString(TEXT("∞")));
		}
	}
	else
	{
		const float AmmoPercent = (InMaxAmmo > 0) ? (float)InCurrentAmmo / (float)InMaxAmmo : 0.0f;
		if (ProgressBar_Ammo)
		{
			ProgressBar_Ammo->SetPercent(AmmoPercent);
		}
		if (Text_AmmoValue)
		{
			Text_AmmoValue->SetText(FText::Format(FText::FromString(TEXT("{0} / {1}")), FText::AsNumber(InCurrentAmmo), FText::AsNumber(InMaxAmmo)));
		}
	}
}

void UPEV2_GameHUDWidget::UpdateAbility(UPEV2_AbilityData* InAbilityData, int32 InRemainingCharges, float InRemainingCooldown)
{
	CachedAbilityData = InAbilityData;
	CachedAbilityCharges = InRemainingCharges;
	CachedRemainingCooldown = InRemainingCooldown;

	if (SlateHUDWidget.IsValid())
	{
		SlateHUDWidget->UpdateAbility(InAbilityData, InRemainingCharges, InRemainingCooldown);
	}

	if (InAbilityData)
	{
		if (Text_AbilityName)
		{
			Text_AbilityName->SetText(InAbilityData->AbilityName);
		}

		if (Text_AbilityCharges)
		{
			Text_AbilityCharges->SetText(FText::Format(FText::FromString(TEXT("x{0}")), FText::AsNumber(InRemainingCharges)));
		}

		if (Image_AbilityIcon)
		{
			if (UTexture2D* Icon = GetOrCreateIconForAbility(InAbilityData, InAbilityData->AbilityName.ToString()))
			{
				Image_AbilityIcon->SetBrushFromTexture(Icon);
			}
		}

		if (Text_AbilityCooldown)
		{
			if (InRemainingCharges > 0 && InRemainingCooldown <= 0.05f)
			{
				Text_AbilityCooldown->SetText(FText::FromString(TEXT("LISTA")));
			}
			else
			{
				Text_AbilityCooldown->SetText(FText::Format(FText::FromString(TEXT("{0}s")), FText::AsNumber(FMath::CeilToInt(InRemainingCooldown))));
			}
		}
	}
	else
	{
		if (Text_AbilityName)
		{
			Text_AbilityName->SetText(FText::FromString(TEXT("HABILIDAD [VACÍO]")));
		}
		if (Text_AbilityCooldown)
		{
			Text_AbilityCooldown->SetText(FText::FromString(TEXT("NINGUNA")));
		}
		if (Text_AbilityCharges)
		{
			Text_AbilityCharges->SetText(FText::GetEmpty());
		}
	}
}

void UPEV2_GameHUDWidget::UpdateAbilityCooldown(float InRemainingCooldown, float InTotalDuration)
{
	CachedRemainingCooldown = InRemainingCooldown;
	CachedTotalCooldown = InTotalDuration;

	if (SlateHUDWidget.IsValid())
	{
		SlateHUDWidget->UpdateAbilityCooldown(InRemainingCooldown, InTotalDuration);
	}

	if (ProgressBar_AbilityCooldown)
	{
		if (InRemainingCooldown > 0.05f && InTotalDuration > 0.0f)
		{
			ProgressBar_AbilityCooldown->SetPercent(FMath::Clamp(1.0f - (InRemainingCooldown / InTotalDuration), 0.0f, 1.0f));
		}
		else
		{
			ProgressBar_AbilityCooldown->SetPercent(1.0f);
		}
	}

	if (Text_AbilityCooldown)
	{
		if (InRemainingCooldown > 0.05f)
		{
			Text_AbilityCooldown->SetText(FText::Format(FText::FromString(TEXT("{0}s")), FText::AsNumber(FMath::CeilToInt(InRemainingCooldown))));
		}
		else if (CachedAbilityCharges > 0)
		{
			Text_AbilityCooldown->SetText(FText::FromString(TEXT("LISTA")));
		}
	}
}

void UPEV2_GameHUDWidget::InitializeRoster(const TArray<TSubclassOf<APEV2_WeaponBase>>& WeaponClasses, int32 ActiveTierIndex)
{
	CachedActiveTier = ActiveTierIndex;
	CachedRosterItems.Empty();

	for (int32 i = 0; i < WeaponClasses.Num(); ++i)
	{
		TSubclassOf<APEV2_WeaponBase> WeaponClass = WeaponClasses[i];
		FPEV2_HUDWeaponItem Item;
		Item.TierIndex = i;
		Item.bIsActive = (i == ActiveTierIndex);

		FString FallbackName = TEXT("Arma");
		if (WeaponClass)
		{
			if (APEV2_WeaponBase* WeaponCDO = WeaponClass->GetDefaultObject<APEV2_WeaponBase>())
			{
				Item.WeaponName = WeaponCDO->GetWeaponName();
				Item.WeaponIcon = WeaponCDO->GetWeaponIcon();
				FallbackName = WeaponCDO->GetName();
			}
		}

		if (Item.WeaponName.IsEmpty())
		{
			switch (i)
			{
			case 0: Item.WeaponName = FText::FromString(TEXT("Pistola")); break;
			case 1: Item.WeaponName = FText::FromString(TEXT("MGP-7")); break;
			case 2: Item.WeaponName = FText::FromString(TEXT("ZC-57")); break;
			case 3: Item.WeaponName = FText::FromString(TEXT("Lanzagranadas")); break;
			case 4: Item.WeaponName = FText::FromString(TEXT("Cuchillo")); break;
			default: Item.WeaponName = FText::Format(FText::FromString(TEXT("Nivel {0}")), FText::AsNumber(i + 1)); break;
			}
		}

		if (!Item.WeaponIcon)
		{
			Item.WeaponIcon = GetOrCreateIconForWeapon(WeaponClass, FallbackName);
		}

		CachedRosterItems.Add(Item);
	}

	if (SlateHUDWidget.IsValid())
	{
		SlateHUDWidget->SetRosterItems(CachedRosterItems, ActiveTierIndex);
	}
}

void UPEV2_GameHUDWidget::SetActiveWeaponTier(int32 ActiveTierIndex)
{
	CachedActiveTier = ActiveTierIndex;

	if (CachedRosterItems.Num() == 0)
	{
		if (APEV2_Character* OwnerChar = Cast<APEV2_Character>(GetOwningPlayerPawn()))
		{
			if (OwnerChar->GetWeaponTierClasses().Num() > 0)
			{
				InitializeRoster(OwnerChar->GetWeaponTierClasses(), ActiveTierIndex);
				return;
			}
		}
	}

	for (FPEV2_HUDWeaponItem& Item : CachedRosterItems)
	{
		Item.bIsActive = (Item.TierIndex == ActiveTierIndex);
	}

	if (SlateHUDWidget.IsValid())
	{
		SlateHUDWidget->SetActiveTier(ActiveTierIndex);
	}
}

UTexture2D* UPEV2_GameHUDWidget::GetOrCreateIconForWeapon(TSubclassOf<APEV2_WeaponBase> WeaponClass, const FString& FallbackName)
{
	if (WeaponClass)
	{
		if (APEV2_WeaponBase* WeaponCDO = WeaponClass->GetDefaultObject<APEV2_WeaponBase>())
		{
			if (WeaponCDO->GetWeaponIcon())
			{
				return WeaponCDO->GetWeaponIcon();
			}
		}
	}

	const FString Key = WeaponClass ? WeaponClass->GetName() : FallbackName;
	if (TObjectPtr<UTexture2D>* Existing = StaticWeaponIcons.Find(Key))
	{
		if (Existing->Get())
		{
			return Existing->Get();
		}
	}

	const int32 W = 64;
	const int32 H = 32;
	UTexture2D* NewTex = UTexture2D::CreateTransient(W, H, PF_B8G8R8A8);
	if (!NewTex)
	{
		return nullptr;
	}

	NewTex->NeverStream = true;
	NewTex->SRGB = true;
	NewTex->AddToRoot();

	FTexture2DMipMap& Mip = NewTex->GetPlatformData()->Mips[0];
	uint8* Pixels = (uint8*)Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memzero(Pixels, W * H * 4);

	auto DrawRect = [&](int32 X0, int32 Y0, int32 X1, int32 Y1)
	{
		for (int32 Y = FMath::Clamp(Y0, 0, H - 1); Y <= FMath::Clamp(Y1, 0, H - 1); ++Y)
		{
			for (int32 X = FMath::Clamp(X0, 0, W - 1); X <= FMath::Clamp(X1, 0, W - 1); ++X)
			{
				const int32 Idx = (Y * W + X) * 4;
				Pixels[Idx + 0] = 255;
				Pixels[Idx + 1] = 255;
				Pixels[Idx + 2] = 255;
				Pixels[Idx + 3] = 255;
			}
		}
	};

	const FString LowerKey = Key.ToLower();

	if (LowerKey.Contains(TEXT("knife")) || LowerKey.Contains(TEXT("cuchillo")))
	{
		DrawRect(22, 13, 50, 17);
		DrawRect(50, 14, 53, 16);
		DrawRect(53, 15, 55, 15);
		DrawRect(20, 8, 23, 22);
		DrawRect(8, 12, 20, 18);
		DrawRect(5, 11, 8, 19);
	}
	else if (LowerKey.Contains(TEXT("launcher")) || LowerKey.Contains(TEXT("lanzagranadas")))
	{
		DrawRect(28, 9, 52, 17);
		DrawRect(52, 8, 55, 18);
		DrawRect(18, 7, 28, 19);
		DrawRect(8, 11, 18, 15);
		DrawRect(14, 15, 18, 24);
		DrawRect(38, 17, 42, 24);
	}
	else if (LowerKey.Contains(TEXT("smg")) || LowerKey.Contains(TEXT("subfusil")) || LowerKey.Contains(TEXT("mgp")))
	{
		DrawRect(14, 11, 48, 15);
		DrawRect(48, 12, 53, 14);
		DrawRect(32, 15, 37, 27);
		DrawRect(20, 15, 25, 25);
		DrawRect(6, 11, 14, 14);
		DrawRect(6, 14, 9, 20);
		DrawRect(18, 9, 20, 11);
		DrawRect(44, 9, 46, 11);
	}
	else if (LowerKey.Contains(TEXT("rifle")) || LowerKey.Contains(TEXT("fusil")) || LowerKey.Contains(TEXT("zc57")) || LowerKey.Contains(TEXT("zc-57")))
	{
		DrawRect(16, 11, 52, 15);
		DrawRect(52, 10, 56, 16);
		DrawRect(20, 8, 34, 11);
		DrawRect(32, 15, 38, 21);
		DrawRect(34, 21, 40, 26);
		DrawRect(22, 15, 27, 25);
		DrawRect(6, 11, 16, 16);
		DrawRect(6, 16, 10, 21);
	}
	else
	{
		DrawRect(18, 10, 48, 15);
		DrawRect(48, 11, 50, 14);
		DrawRect(46, 8, 48, 10);
		DrawRect(20, 8, 22, 10);
		DrawRect(20, 15, 27, 27);
		DrawRect(18, 20, 25, 28);
		DrawRect(27, 18, 35, 19);
		DrawRect(34, 15, 35, 19);
		DrawRect(30, 16, 31, 18);
	}

	Mip.BulkData.Unlock();
	NewTex->UpdateResource();

	StaticWeaponIcons.Add(Key, NewTex);
	return NewTex;
}

UTexture2D* UPEV2_GameHUDWidget::GetOrCreateIconForAbility(UPEV2_AbilityData* AbilityData, const FString& FallbackName)
{
	if (AbilityData && AbilityData->AbilityIcon)
	{
		return AbilityData->AbilityIcon;
	}

	const FString Key = AbilityData ? AbilityData->GetName() : FallbackName;
	if (TObjectPtr<UTexture2D>* Existing = StaticAbilityIcons.Find(Key))
	{
		if (Existing->Get())
		{
			return Existing->Get();
		}
	}

	const int32 W = 32;
	const int32 H = 32;
	UTexture2D* NewTex = UTexture2D::CreateTransient(W, H, PF_B8G8R8A8);
	if (!NewTex)
	{
		return nullptr;
	}

	NewTex->NeverStream = true;
	NewTex->SRGB = true;
	NewTex->AddToRoot();

	FTexture2DMipMap& Mip = NewTex->GetPlatformData()->Mips[0];
	uint8* Pixels = (uint8*)Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memzero(Pixels, W * H * 4);

	auto DrawRect = [&](int32 X0, int32 Y0, int32 X1, int32 Y1)
	{
		for (int32 Y = FMath::Clamp(Y0, 0, H - 1); Y <= FMath::Clamp(Y1, 0, H - 1); ++Y)
		{
			for (int32 X = FMath::Clamp(X0, 0, W - 1); X <= FMath::Clamp(X1, 0, W - 1); ++X)
			{
				const int32 Idx = (Y * W + X) * 4;
				Pixels[Idx + 0] = 255;
				Pixels[Idx + 1] = 255;
				Pixels[Idx + 2] = 255;
				Pixels[Idx + 3] = 255;
			}
		}
	};

	const FString LowerKey = Key.ToLower();

	if (LowerKey.Contains(TEXT("dash")))
	{
		DrawRect(4, 11, 8, 12);
		DrawRect(2, 15, 7, 16);
		DrawRect(4, 19, 8, 20);

		for (int32 i = 0; i < 8; ++i)
		{
			DrawRect(9 + i, 8 + i, 11 + i, 8 + i);
			DrawRect(16 - i, 16 + i, 18 - i, 16 + i);

			DrawRect(17 + i, 8 + i, 19 + i, 8 + i);
			DrawRect(24 - i, 16 + i, 26 - i, 16 + i);
		}
	}
	else if (LowerKey.Contains(TEXT("grenade")) || LowerKey.Contains(TEXT("granada")))
	{
		DrawRect(11, 13, 21, 25);
		DrawRect(9, 15, 23, 23);
		DrawRect(13, 9, 19, 12);
		DrawRect(8, 7, 11, 10);
		DrawRect(20, 10, 23, 20);
	}
	else if (LowerKey.Contains(TEXT("shield")) || LowerKey.Contains(TEXT("escudo")))
	{
		DrawRect(8, 6, 24, 9);
		DrawRect(7, 10, 25, 17);
		DrawRect(9, 18, 23, 22);
		DrawRect(12, 23, 20, 25);
		DrawRect(15, 26, 17, 27);
	}
	else if (LowerKey.Contains(TEXT("speed")) || LowerKey.Contains(TEXT("velocidad")))
	{
		DrawRect(16, 5, 20, 12);
		DrawRect(12, 11, 22, 15);
		DrawRect(11, 15, 18, 26);
	}
	else
	{
		DrawRect(10, 15, 22, 17);
	}

	Mip.BulkData.Unlock();
	NewTex->UpdateResource();

	StaticAbilityIcons.Add(Key, NewTex);
	return NewTex;
}
