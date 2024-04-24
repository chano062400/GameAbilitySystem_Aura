#include "UI/HUD/LoadScreenHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/Widgets/LoadScreenWidget.h"
#include "UI/MVVM_LoadScreen.h"

void ALoadScreenHUD::BeginPlay()
{
	Super::BeginPlay();

	LoadScreenViewModel = NewObject<UMVVM_LoadScreen>(this, LoadScreenViewModelClass);
	LoadScreenViewModel->InitializeLoadSlot();

	LoadScreenWidget = CreateWidget<ULoadScreenWidget>(GetWorld(), LoadScreenWidgetClass);
	LoadScreenWidget->AddToViewport();
	LoadScreenWidget->BlueprintInitializeWidget();
}