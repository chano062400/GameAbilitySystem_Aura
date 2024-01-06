#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Aura/Public/AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Interaction/CombatInterface.h"
#include "AuraAbilityTypes.h"
#include "AbilitySystem/AuraAbilitySystemGlobals.h"

//Raw Struct라서 Blueprint나 Reflection System 어느 곳에도 노출시키지 않을 것이라서 F 안붙임.
struct AuraDamagestatics
{
	//존재하지 않는 변수를 넣어도 알아서 FProperty형 포인터로 만들어서 CaptureDefinition으로 만들어 줌.
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor)
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration)
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage)

	AuraDamagestatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Source, false);

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
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;
	TScriptInterface<ICombatInterface> SourceCombatInterface = TScriptInterface<ICombatInterface>(SourceAvatar);
	TScriptInterface<ICombatInterface> TargetCombatInterface = TScriptInterface<ICombatInterface>(TargetAvatar);

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// GetAggregatedTags() - /** Returns combination of spec and actor tags */
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	//FAggregatorEvaluateParameters - /** Data that is used in aggregator evaluation that is passed from the caller/game code */
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	//Get Damage Set By Caller Magnitude , 찾을 수 없다면 0.f를 반환.
	float Damage = 0.f;
	for (const auto& DamageTypeTag : FAuraGameplayTags::Get().DamageTypesToResistances)
	{
		const float DamageTypeValue = Spec.GetSetByCallerMagnitude(DamageTypeTag.Key); // {DamageType, Resistance}
		Damage += DamageTypeValue;
	}

	//Target의 BlockChance를 캡처. Block 한다면 Damage는 1/2이 됨.
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluateParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(0.f, TargetBlockChance);

	//1 ~ 100 랜덤 값보다 Target의 BlockChange가 크다면 Block에 성공.
	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;
	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
	UAuraAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, bBlocked);

	Damage = bBlocked ? Damage / 2.f : Damage;

	//Target의 Armor 값
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluateParameters, TargetArmor);
	TargetArmor = FMath::Max<float>(0.f, TargetArmor);

	//Source의 Armor Pentration 값. Armor Penetration은 Target의 Armor의 percent를 무시 함.
	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvaluateParameters, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(0.f, SourceArmorPenetration);

	const UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
	FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("ArmorPenetration"), FString());
	/** Evaluate this curve at the specified time */
	const float ArmorPenetrationCoefficients = ArmorPenetrationCurve->Eval(SourceCombatInterface->GetPlayerLevel());

	const FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("EffectiveArmor"), FString());
	const float EffectiveArmorCoefficients = EffectiveArmorCurve->Eval(TargetCombatInterface->GetPlayerLevel());

	//ArmorPenetration이 500이어야 트루뎀이 들어감.
	const float EffectiveArmor = TargetArmor * (100 - SourceArmorPenetration * ArmorPenetrationCoefficients) / 100.f;
	//EffectiveArmor 4 당 1%의 데미지 감소 효과를 봄.
	Damage *= (100 - EffectiveArmor * EffectiveArmorCoefficients) / 100.f;

	float SourceCriticalHitChnace = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluateParameters, SourceCriticalHitChnace);
	SourceCriticalHitChnace = FMath::Max<float>(0.f, SourceCriticalHitChnace);

	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef, EvaluateParameters, TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max<float>(0.f, TargetCriticalHitResistance);

	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef, EvaluateParameters, SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(0.f, SourceCriticalHitDamage);
	
	const FRealCurve* CriticalHitResistanceCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("CriticalHitResistance"), FString());
	const float CriticalHitResistanceCoefficients = CriticalHitResistanceCurve->Eval(TargetCombatInterface->GetPlayerLevel());

	//CriticalHitResistence는 CriticalHitChance를 일정 감소시킴. 
	const float EffectiveCriticalHitChance = SourceCriticalHitChnace - CriticalHitResistanceCoefficients;
	const bool bCriticalHIt = EffectiveCriticalHitChance > FMath::RandRange(1, 100);
	UAuraAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHIt);


	// 치명타가 발생하면 데미지는 1.25 * CriticalHitDamage 만큼의 추가 피해를 입힘.
	Damage = bCriticalHIt ? 1.25f * Damage * SourceCriticalHitDamage : Damage;

	const FGameplayModifierEvaluatedData EvaluateData(UAuraAttributeSet::GetInComingDamageAttribute(), EGameplayModOp::Additive, Damage);
	//AddOutputModifier - /** Add the specified evaluated data to the execution's output modifiers */
	OutExecutionOutput.AddOutputModifier(EvaluateData);
}
