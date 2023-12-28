#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

//Raw Struct라서 Blueprint나 Reflection System 어느 곳에도 노출시키지 않을 것이라서 F 안붙임.
struct AuraDamagestatics
{
	//존재하지 않는 변수를 넣어도 알아서 FProperty형 포인터로 만들어서 CaptureDefinition으로 만들어 줌.
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
	//Calculation과 관련있는 Capture할 Attribute를 등록.
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
	++Armor; // Armor가 0일 수도 있기 때문에 1을 더해주어 확인.

	const FGameplayModifierEvaluatedData EvaluateData(DamageStatics().ArmorProperty, EGameplayModOp::Additive, Armor);
	//AddOutputModifier - /** Add the specified evaluated data to the execution's output modifiers */
	OutExecutionOutput.AddOutputModifier(EvaluateData);
}
