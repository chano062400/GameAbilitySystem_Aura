#include "UI/HUD/AuraHUD.h"
#include "UI/Widgets/AuraUserWidget.h"
#include "UI/WidgetController/OverlayWidgetController.h"

//OverlayWidgetController�� �������� �ʾҴٸ� �����ϰ�, �������ִٸ� �����͸� ��ȯ.
UOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass); // OverlayWidgetController�� �����ϴ� ��ü , OverlayWidgetController�� Ŭ����
		OverlayWidgetController->SetWidgetControllerParams(WCParams); // WidgetController�� ��������� �ʱ�ȭ.

		return OverlayWidgetController;
	}

	return OverlayWidgetController;
}

//UserWidget�� �����ϰ�, �ش� WidgetController���� ��� �����ϰ� OverlayWidget�� OverlayWidgetController�� ����. 
void AAuraHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("OverlayWidgetClass unInitialized, Please fill out BP_AuraHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("OverlayWidgetControllerClass unInitialized, Please fill out BP_AuraHUD"));

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UAuraUserWidget>(Widget);
	
	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);
	
	OverlayWidget->SetWidgetController(WidgetController);

	Widget->AddToViewport();
}
