#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

//Raw Struct�� Blueprint�� Reflection System ��� ������ �����Ű�� ���� ���̶� F �Ⱥ���.
struct AuraDamagestatics
{
	//�������� �ʴ� ������ �־ �˾Ƽ� FProperty�� �����ͷ� ���� CaptureDefinition���� ����� ��.
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor)

	AuraDamagestatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
	}
};

static const AuraDamagestatics& DamageStatics()
{
	static AuraDamagestatics DStatics;

	return DStatics;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	//Calculation�� �����ִ� Capture�� Attribute�� ���.
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	const AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	const AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// GetAggregatedTags() - /** Returns combination of spec and actor tags */
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	//FAggregatorEvaluateParameters - /** Data that is used in aggregator evaluation that is passed from the caller/game code */
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float Armor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluateParameters, Armor);
	Armor = FMath::Max<float>(0.f, Armor);
	++Armor; // Armor�� 0�� ���� �ֱ� ������ 1�� �����־� Ȯ��.

	const FGameplayModifierEvaluatedData EvaluateData(DamageStatics().ArmorProperty, EGameplayModOp::Additive, Armor);
	//AddOutputModifier - /** Add the specified evaluated data to the execution's output modifiers */
	OutExecutionOutput.AddOutputModifier(EvaluateData);
}
