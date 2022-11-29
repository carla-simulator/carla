// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OpenDriveToMap.generated.h"

/**
 * 
 */
UCLASS()
class CARLATOOLS_API UOpenDriveToMap : public UUserWidget
{
  GENERATED_BODY()
  
public:

  UPROPERTY( meta = (BindWidget) )
  class UButton* ChooseFileButon;

protected:
  virtual void NativeConstruct() override;
  virtual void NativeDestruct() override;
private:
  UFUNCTION()
  void OpenFileDialog();
};
