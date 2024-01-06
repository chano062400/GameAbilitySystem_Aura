
#include "AuraGameplayTags.h"
#include "GameplayTagsManager.h"

FAuraGameplayTags FAuraGameplayTags::GameplayTags; //정적변수는 cpp파일에서 유형을 명시적으로 선언해줘야 함.

void FAuraGameplayTags::InitializeNativeGameplayTags()
{
	//GameplayTag를 생성하면 GameplayTagManager에 등록됨 , GameplayTagManager에 NatvieGameplayTag를 등록(이름,설명)

	GameplayTags.Attributes_Primary_Strength = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Strength"), 
		FString("Increases Physical Damages")
	);

	GameplayTags.Attributes_Primary_Intelligence = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Intelligence"),
		FString("Increases Magical Damages")
	);

	GameplayTags.Attributes_Primary_Resilience = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Resilience"),
		FString("Increases Armor and Armor Penetration")
	);

	GameplayTags.Attributes_Primary_Vigor = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Vigor"),
		FString("Increases Health")
	);

	/* Secondary Attributes */

	GameplayTags.Attributes_Secondary_Armor = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.Armor"), 
		FString("Reduces Damage Taken, Improves Block Chance")
	); 

	GameplayTags.Attributes_Secondary_ArmorPenetration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.ArmorPenetration"), 
		FString("Ignored Percentage of enemy Armor, Increases Critical Hit Chance")
	); 
	
	GameplayTags.Attributes_Secondary_BlockChance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.BlockChance"), 
		FString("Increases Chance of Block")
	); 

	GameplayTags.Attributes_Secondary_CriticalHitChance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitChance"),
		FString("Chance to double damage plus Critical Hit Bonus")
	);

	GameplayTags.Attributes_Secondary_CriticalHitDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitDamage"),
		FString("Bonus Damage added when a Critical Hit is Scored")
	);

	GameplayTags.Attributes_Secondary_CriticalHitResistence = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitResistence"),
		FString("Reduces Critical Hit Chance of Attacking Enenies")
	);

	GameplayTags.Attributes_Secondary_HealthRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.HealthRegeneration"),
		FString("Amount of Health Regenerated every 1 second")
	);

	GameplayTags.Attributes_Secondary_ManaRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.ManaRegeneration"),
		FString("Amount of Mana Regenerated every 1 second")
	);

	GameplayTags.Attributes_Secondary_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxHealth"),
		FString("Maximum Amount of Health Obtainable")
	);

	GameplayTags.Attributes_Secondary_MaxMana = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxMana"),
		FString("Maximum Amount of Mana Obtainable")
	);

	/* Input Tags*/

	GameplayTags.InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.LMB"),
		FString("Input Tag for Left Mouse Button")
	);
	
	GameplayTags.InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.RMB"),
		FString("Input Tag for Reft Mouse Button")
	);
	
	GameplayTags.InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.1"),
		FString("Input Tag for 1 Key")
	);
	
	GameplayTags.InputTag_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.2"),
		FString("Input Tag for 2 Key")
	);
	
	GameplayTags.InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.3"),
		FString("Input Tag for 3 Key")
	);
	
	GameplayTags.InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.4"),
		FString("Input Tag for 4 Key")
	);

	// Damage Types

	GameplayTags.Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage"),
		FString("Damage")
	);
	
	GameplayTags.Damage_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Fire"),
		FString("Fire Damage Type")
	);
		
	GameplayTags.Damage_Lightning = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Fire"),
		FString("Lightning Damage Type")
	);
		
	GameplayTags.Damage_Arcane = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Fire"),
		FString("Arcane Damage Type")
	);
		
	GameplayTags.Damage_Physical = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Fire"),
		FString("Physical Damage Type")
	);

	// Resistance

	GameplayTags.Attributes_Resistance_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.Fire"),
		FString("Resistance to Fire Damage ")
	);
	
	GameplayTags.Attributes_Resistance_Lightning = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.Lightning"),
		FString("Resistance to Lightning Damage ")
	);
	
	GameplayTags.Attributes_Resistance_Arcane = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.Arcane"),
		FString("Resistance to Arcane Damage ")
	);
	
	GameplayTags.Attributes_Resistance_Physical = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.Physical"),
		FString("Resistance to Physical Damage ")
	);

	// 각 Damage Type에 대응하는 Resistance를 가진 Map.
	
	GameplayTags.DamageTypesToResistances.Add({ GameplayTags.Damage_Fire, GameplayTags.Attributes_Resistance_Fire });
	GameplayTags.DamageTypesToResistances.Add({ GameplayTags.Damage_Lightning, GameplayTags.Attributes_Resistance_Lightning });
	GameplayTags.DamageTypesToResistances.Add({ GameplayTags.Damage_Arcane, GameplayTags.Attributes_Resistance_Arcane });
	GameplayTags.DamageTypesToResistances.Add({ GameplayTags.Damage_Physical, GameplayTags.Attributes_Resistance_Physical });

	GameplayTags.Effect_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Effect.HitReact"),
		FString("Hit React")
	);


}
