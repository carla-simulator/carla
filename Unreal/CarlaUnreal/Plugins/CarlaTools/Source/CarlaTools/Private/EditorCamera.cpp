// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "EditorCamera.h"

#include <util/ue-header-guard-begin.h>
#include "UnrealClient.h"
#include "Editor/EditorEngine.h"
#include "EditorViewportClient.h"
#include <util/ue-header-guard-end.h>

void AEditorCameraUtils::Get()
{
    auto ViewportClient = dynamic_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
    CameraTransform = FTransform();
    CameraTransform.SetLocation(ViewportClient->GetViewLocation());
    CameraTransform.SetRotation(FQuat(ViewportClient->GetViewRotation()));
}

void AEditorCameraUtils::Set()
{
    auto ViewportClient = dynamic_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
    ViewportClient->SetViewLocation(CameraTransform.GetLocation());
    ViewportClient->SetViewRotation(FRotator(CameraTransform.GetRotation()));
}
