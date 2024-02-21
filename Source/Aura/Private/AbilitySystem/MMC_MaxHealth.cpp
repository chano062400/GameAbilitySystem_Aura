#include "AbilitySystem/MMC_MaxHealth.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute(); // Capture�� Attribute.
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target; //Target���Լ� Vigor�� Capture�� ��.
	VigorDef.bSnapshot = false; //Caputre�� Ÿ�ֿ̹� ����(Effect Spec�� �������ڸ��� Capture X / Effect Spec�� ����� �� Capture O)

	RelevantAttributesToCapture.Add(VigorDef); //Capture�� Attribute���� ��ȯ
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	//Capture�� ������ ������ ����.
	//Source or Target���Լ� Tag�� ������.
	
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluationParameters, Vigor);
	Vigor = FMath::Max<float>(Vigor, 0.f);
	
	int32 PlayerLevel = 1;
	if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
	{
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}
	return 80.f + 2.5f * Vigor + 10.f * PlayerLevel;

	return 0.0f;
}
