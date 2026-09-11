// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Commandlet/CarlaDumpBlueprintCommandlet.h"

#include <util/ue-header-guard-begin.h>
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Engine/Blueprint.h"
#include <util/ue-header-guard-end.h>

DEFINE_LOG_CATEGORY_STATIC(LogCarlaDumpBlueprint, Log, All);

int32 UCarlaDumpBlueprintCommandlet::Main(const FString& Params)
{
  TArray<FString> Tokens;
  TArray<FString> Switches;
  ParseCommandLine(*Params, Tokens, Switches);

  FString AssetPath;
  for (const FString& Token : Tokens)
  {
    if (Token.StartsWith(TEXT("/")))
    {
      AssetPath = Token;
      break;
    }
  }
  if (AssetPath.IsEmpty())
  {
    UE_LOG(LogCarlaDumpBlueprint, Error, TEXT("Usage: -run=CarlaDumpBlueprint <ObjectPath>"));
    return 1;
  }

  UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *AssetPath);
  if (Blueprint == nullptr)
  {
    UE_LOG(LogCarlaDumpBlueprint, Error, TEXT("Failed to load blueprint '%s'"), *AssetPath);
    return 1;
  }

  TArray<UEdGraph*> Graphs;
  Blueprint->GetAllGraphs(Graphs);
  UE_LOG(LogCarlaDumpBlueprint, Display, TEXT("BPDUMP BEGIN %s graphs=%d"), *AssetPath, Graphs.Num());

  for (UEdGraph* Graph : Graphs)
  {
    if (Graph == nullptr)
    {
      continue;
    }
    UE_LOG(LogCarlaDumpBlueprint, Display, TEXT("BPDUMP GRAPH %s nodes=%d"), *Graph->GetName(), Graph->Nodes.Num());
    for (UEdGraphNode* Node : Graph->Nodes)
    {
      if (Node == nullptr)
      {
        continue;
      }
      const FString Title = Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString().Replace(TEXT("\n"), TEXT(" | "));
      UE_LOG(LogCarlaDumpBlueprint, Display, TEXT("BPDUMP  NODE id=%s class=%s title=\"%s\""),
          *Node->GetName(), *Node->GetClass()->GetName(), *Title);
      for (UEdGraphPin* Pin : Node->Pins)
      {
        if (Pin == nullptr)
        {
          continue;
        }
        FString Links;
        for (UEdGraphPin* Linked : Pin->LinkedTo)
        {
          if (Linked != nullptr && Linked->GetOwningNode() != nullptr)
          {
            Links += FString::Printf(TEXT("%s.%s;"),
                *Linked->GetOwningNode()->GetName(), *Linked->PinName.ToString());
          }
        }
        const bool bHasDefault =
            !Pin->DefaultValue.IsEmpty() || Pin->DefaultObject != nullptr || !Pin->DefaultTextValue.IsEmpty();
        if (bHasDefault || !Links.IsEmpty())
        {
          UE_LOG(LogCarlaDumpBlueprint, Display, TEXT("BPDUMP   PIN %s %s type=%s def=\"%s\" obj=%s -> %s"),
              Pin->Direction == EGPD_Input ? TEXT("in") : TEXT("out"),
              *Pin->PinName.ToString(),
              *Pin->PinType.PinCategory.ToString(),
              *Pin->DefaultValue,
              Pin->DefaultObject != nullptr ? *Pin->DefaultObject->GetName() : TEXT("null"),
              *Links);
        }
      }
    }
  }

  UE_LOG(LogCarlaDumpBlueprint, Display, TEXT("BPDUMP END"));
  return 0;
}
