#include "AbilitySystem/MMC_MaxHealth.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute(); // Capture할 Attribute.
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target; //Target에게서 Vigor를 Capture할 것.
	VigorDef.bSnapshot = false; //Caputre할 타이밍에 연관(Effect Spec이 생성되자마자 Capture X / Effect Spec이 적용될 때 Capture O)

	RelevantAttributesToCapture.Add(VigorDef); //Capture할 Attribute들을 반환
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	//Capture된 변수를 엑세스 가능.
	//Source or Target에게서 Tag를 가져옴.
	
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
