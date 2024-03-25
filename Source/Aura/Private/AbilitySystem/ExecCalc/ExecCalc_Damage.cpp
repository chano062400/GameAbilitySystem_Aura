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

	//Resistance
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance)

	AuraDamagestatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Source, false);

		//Resistance
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, FireResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, LightningResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArcaneResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, PhysicalResistance, Target, false);
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

	//Resistance
	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArcaneResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalResistanceDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;

	const FAuraGameplayTags& Tag = FAuraGameplayTags::Get();
	TagsToCaptureDefs.Add(Tag.Attributes_Secondary_Armor, DamageStatics().ArmorDef);
	TagsToCaptureDefs.Add(Tag.Attributes_Secondary_ArmorPenetration, DamageStatics().ArmorPenetrationDef);
	TagsToCaptureDefs.Add(Tag.Attributes_Secondary_BlockChance, DamageStatics().BlockChanceDef);
	TagsToCaptureDefs.Add(Tag.Attributes_Secondary_CriticalHitChance, DamageStatics().CriticalHitChanceDef);
	TagsToCaptureDefs.Add(Tag.Attributes_Secondary_CriticalHitResistance, DamageStatics().CriticalHitResistanceDef);
	TagsToCaptureDefs.Add(Tag.Attributes_Secondary_CriticalHitDamage, DamageStatics().CriticalHitDamageDef);

	TagsToCaptureDefs.Add(Tag.Attributes_Resistance_Fire, DamageStatics().FireResistanceDef);
	TagsToCaptureDefs.Add(Tag.Attributes_Resistance_Lightning, DamageStatics().LightningResistanceDef);
	TagsToCaptureDefs.Add(Tag.Attributes_Resistance_Arcane, DamageStatics().ArcaneResistanceDef);
	TagsToCaptureDefs.Add(Tag.Attributes_Resistance_Physical, DamageStatics().PhysicalResistanceDef);

	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	int32 SourcePlayerLevel = 1;
	if (IsValid(SourceAvatar) && SourceAvatar->Implements<UCombatInterface>())
	{
		SourcePlayerLevel = ICombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}
	
	int32 TargetPlayerLevel = 1;
	if (IsValid(TargetAvatar) && TargetAvatar->Implements<UCombatInterface>())
	{
		TargetPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(TargetAvatar);
	}

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// GetAggregatedTags() - /** Returns combination of spec and actor tags */
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	//FAggregatorEvaluateParameters - /** Data that is used in aggregator evaluation that is passed from the caller/game code */
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	//Debuff

	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();

	for (TTuple<FGameplayTag, FGameplayTag> Pair : FAuraGameplayTags::Get().DamageTypesToDebuffs)
	{
		const FGameplayTag& DamageType = Pair.Key;
		const FGameplayTag& DebuffType = Pair.Value;
		// 찾지 못하는 경우 -1 반환.
		const float TypeDamage = Spec.GetSetByCallerMagnitude(DamageType, false, -1);
		if (TypeDamage > -.5f)
		{
			// 디버프 적용 
			const float SourceDebuffChance = Spec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Debuff_Chance, false, -1.f);

			float TargetDebuffResistance = 0.f;
			const FGameplayTag& ResistanceTag = FAuraGameplayTags::Get().DamageTypesToResistances[DamageType];
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(TagsToCaptureDefs[ResistanceTag], EvaluateParameters, TargetDebuffResistance);
			TargetDebuffResistance = FMath::Max<float>(TargetDebuffResistance, 0.f);

			const float EffectiveDebuffChance = SourceDebuffChance * (100 - TargetDebuffResistance) / 100.f;

			const bool bApplyDebuff = FMath::RandRange(1, 100) < EffectiveDebuffChance;
			if (bApplyDebuff)
			{
				UAuraAbilitySystemLibrary::SetIsSuccessDebuff(EffectContextHandle, true);
				UAuraAbilitySystemLibrary::SetDamageType(EffectContextHandle, DamageType);

				const float DebuffDamage = Spec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Debuff_Damage, false, -1.f);
				const float DebuffDuration = Spec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Debuff_Duration, false, -1.f);
				const float DebuffFrequency = Spec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Debuff_Frequency, false, -1.f);

				UAuraAbilitySystemLibrary::SetDebuffDamage(EffectContextHandle, DebuffDamage);
				UAuraAbilitySystemLibrary::SetDebuffDuration(EffectContextHandle, DebuffDuration);
				UAuraAbilitySystemLibrary::SetDebuffFrequency(EffectContextHandle, DebuffFrequency);


			}
		}
	}

	//Get Damage Set By Caller Magnitude , 찾을 수 없다면 0.f를 반환.
	float Damage = 0.f;
	for (const TTuple<FGameplayTag, FGameplayTag>& Pair : FAuraGameplayTags::Get().DamageTypesToResistances)
	{
		const FGameplayTag DamageType = Pair.Key;
		const FGameplayTag ResistanceType = Pair.Value;

		checkf(TagsToCaptureDefs.Contains(ResistanceType), TEXT("TagsToCaptureDefs doesn't containg Tag : [%s] in ExecCalc_Damage"), *ResistanceType.ToString());

		const FGameplayEffectAttributeCaptureDefinition ResistanceCaptureDef = TagsToCaptureDefs[ResistanceType]; //TagsToCaptureDefs Map에서 Resistance Tag에 맞는 CaptureDef를 가져옴

		float DamageTypeValue = Spec.GetSetByCallerMagnitude(DamageType, false, 0.0f);

		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(ResistanceCaptureDef, EvaluateParameters, Resistance); 
		Resistance = FMath::Clamp(Resistance, 0.f, 100.f);

		Damage += DamageTypeValue * ((100.f - Resistance) / 100.f);
	}

	//Target의 BlockChance를 캡처. Block 한다면 Damage는 1/2이 됨.
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluateParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(0.f, TargetBlockChance);

	//1 ~ 100 랜덤 값보다 Target의 BlockChange가 크다면 Block에 성공.
	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;
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
	const float ArmorPenetrationCoefficients = ArmorPenetrationCurve->Eval(SourcePlayerLevel);

	const FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("EffectiveArmor"), FString());
	const float EffectiveArmorCoefficients = EffectiveArmorCurve->Eval(TargetPlayerLevel);

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
	const float CriticalHitResistanceCoefficients = CriticalHitResistanceCurve->Eval(TargetPlayerLevel);

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
