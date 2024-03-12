#include "AuraAbilityTypes.h"

bool FAuraGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	uint32 RepBits = 0; // 8개 이상을 저장할 것이라 uint32로 선언.
	if (Ar.IsSaving())
	{
		if (bReplicateInstigator && Instigator.IsValid())
		{
			RepBits |= 1 << 0; // 오른쪽에서 0번째 bit는 Instiagtor를 의미.
		}
		if (bReplicateEffectCauser && EffectCauser.IsValid())
		{
			RepBits |= 1 << 1; // 오른쪽에서 1번째 bit는 EffectCauser를 의미.
		}
		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}
		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}
		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}
		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}
		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}
		if (bIsBlockedHit)
		{
			RepBits |= 1 << 7; // 오른쪽에서 7번째 bit는 IsBlockedHit를 의미.
		}
		if (bIsCriticalHit)
		{
			RepBits |= 1 << 8; // 오른쪽에서 8번째 bit는 IsCriticalHit를 의미.
		}
		if (bIsSuccessfulDebuff)
		{
			RepBits |= 1 << 9; // 오른쪽에서 8번째 bit는 IsSuccessfulDebuff를 의미.
		}
		if (DebuffDamage > 0.f)
		{
			RepBits |= 1 << 10;
		}
		if (DebuffDuration > 0.f)
		{
			RepBits |= 1 << 11;
		}
		if (DebuffFrequency > 0.f)
		{
			RepBits |= 1 << 12;
		}
		if (DamageType.IsValid())
		{
			RepBits |= 1 << 13;
		}
	}

	Ar.SerializeBits(&RepBits, 14); // 기존 7에서 7개 더 추가했으므로

	if (RepBits & (1 << 0))
	{
		Ar << Instigator; // RepBits에서 Instigator를 뜻하는 bit true(1)이라면 1 0이라면 0
	}
	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser; // RepBits에서 EffectCauser를 뜻하는 bit가 true(1)이라면 1 0이라면 0
	}
	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}
	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}
	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}
	if (RepBits & (1 << 5))
	{
		if (Ar.IsLoading())
		{
			if (!HitResult.IsValid())
			{
				HitResult = TSharedPtr<FHitResult>(new FHitResult());
			}
		}
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}
	if (RepBits & (1 << 7))
	{
		Ar << bIsBlockedHit;
	}
	if (RepBits & (1 << 8))
	{
		Ar << bIsCriticalHit;
	}
	if (RepBits & (1 << 9))
	{
		Ar << bIsSuccessfulDebuff;
	}
	if (RepBits & (1 << 10))
	{
		Ar << DebuffDamage;
	}
	if (RepBits & (1 << 11))
	{
		Ar << DebuffDuration;
	}
	if (RepBits & (1 << 12))
	{
		Ar << DebuffFrequency;
	}
	if (RepBits & (1 << 13))
	{
		if (IsLoading())
		{
			if (!DamageType.IsValid())
			{
				DamageType = TSharedPtr<FGameplayTag>(new FGameplayTag());
			}
		}
		DamageType->NetSerialize(Ar, Map, bOutSuccess);
	}

	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get()); // Just to initialize InstigatorAbilitySystemComponent
	}

	bOutSuccess = true;
	return true;
}