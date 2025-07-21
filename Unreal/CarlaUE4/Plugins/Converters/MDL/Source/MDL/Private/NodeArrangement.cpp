// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#if WITH_MDL_SDK

#include "NodeArrangement.h"
#include "MaterialExpressions.h"

const int32 BASE_HEIGHT = 28;
const int32 CHAR_HEIGHT = 26;
const int32 DELTA_WIDTH = 50;
const int32 DELTA_HEIGHT = 20;

struct FChildData
{
    UMaterialExpression* Expression;
    int32 InputIndex;
};

struct FInt2
{
    FInt2(int32 _X = 0, int32 _Y = 0) : X(_X), Y(_Y) {}

    int32 X;
    int32 Y;
};

FInt2 operator+(const FInt2& A, const FInt2& B)
{
    return FInt2(A.X + B.X, A.Y + B.Y);
}

FInt2 operator-(const FInt2& A, const FInt2& B)
{
    return FInt2(A.X - B.X, A.Y - B.Y);
}

struct FNodeData
{
    FNodeData(UMaterialExpression* E = nullptr, UMaterialExpression* A = nullptr, int32 L = 0) : Expression(E), Anchor(A), Level(L), Offset(), Size(), BBoxOffset(), BBoxSize() {}

    UMaterialExpression* Expression;	// expression for this Node
    UMaterialExpression* Anchor;		// expression to attach this Node to
    TArray<FChildData> Children;		// expressions to attach to this Node
    int32 Level;						// level from root
    FInt2 Offset;						// relative to parent
    FInt2 Size;							// estimated size of widget
    FInt2 BBoxOffset;					// relative to this (Offset)
    FInt2 BBoxSize;						// estimated size of bounding box
};

static void EstimateSize(FNodeData& Node)
{
    if (Node.Expression)
    {
        TArray<FString> Caption;
        Node.Expression->GetCaption(Caption);
        TArray<FExpressionInput*> Inputs = Node.Expression->GetInputs();
        TArray<FExpressionOutput> Outputs = Node.Expression->GetOutputs();

        int32 RowCount = FMath::Max(Inputs.Num(), Outputs.Num());
        int32 CharCount = 0, CaptionWidth = 0;
        const int32 CharWidth = 8;
        const int32 BaseWidth = 40;

        for (int32 i = 0; i < Caption.Num(); i++)
        {
            CaptionWidth = FMath::Max(CaptionWidth, Caption[i].Len());
        }

        if (Node.Expression->IsA<UMaterialExpressionAdd>() ||
            Node.Expression->IsA<UMaterialExpressionAppendVector>() ||
            Node.Expression->IsA<UMaterialExpressionArctangent2>() ||
            Node.Expression->IsA<UMaterialExpressionCrossProduct>() ||
            Node.Expression->IsA<UMaterialExpressionDivide>() ||
            Node.Expression->IsA<UMaterialExpressionDotProduct>() ||
            Node.Expression->IsA<UMaterialExpressionFmod>() ||
            Node.Expression->IsA<UMaterialExpressionMax>() ||
            Node.Expression->IsA<UMaterialExpressionMin>() ||
            Node.Expression->IsA<UMaterialExpressionMultiply>() ||
            Node.Expression->IsA<UMaterialExpressionSubtract>())
        {
            CharCount = 1;
        }
        else if (Node.Expression->IsA<UMaterialExpressionClamp>() ||
            Node.Expression->IsA<UMaterialExpressionTextureProperty>())
        {
            CharCount = 3;
        }
        else if (Node.Expression->IsA<UMaterialExpressionCustom>())
        {
            UMaterialExpressionCustom* Custom = Cast<UMaterialExpressionCustom>(Node.Expression);
            check((CaptionWidth == Custom->Description.Len()) && (CharCount == 0));
            for (int32 i = 0; i < Custom->Inputs.Num(); i++)
            {
                CharCount = FMath::Max(CharCount, Custom->Inputs[i].InputName.ToString().Len());
            }
            check(0 < CharCount);
        }
        else if (Node.Expression->IsA<UMaterialExpressionIf>() ||
            Node.Expression->IsA<UMaterialExpressionPower>())
        {
            CharCount = 4;
        }
        else if (Node.Expression->IsA<UMaterialExpressionLinearInterpolate>())
        {
            CharCount = 5;
        }
        else if (Node.Expression->IsA<UMaterialExpressionNormalize>() ||
            Node.Expression->IsA<UMaterialExpressionNoise>())
        {
            CharCount = 11;
        }
        else if (Node.Expression->IsA<UMaterialExpressionTextureSample>() ||
            Node.Expression->IsA<UMaterialExpressionVectorParameter>())
        {
            CharCount = 15;   // estimated width of the texture/parameter quad
        }
        else if (Node.Expression->IsA<UMaterialExpressionFresnel>())
        {
            CharCount = 21;
        }
        else if (Node.Expression->IsA<UMaterialExpressionMaterialFunctionCall>())
        {
            int32 maxInputLen = 0, maxOutputLen = 0;
            for (int32 i = 0; i < Inputs.Num(); i++)
            {
                maxInputLen = FMath::Max(maxInputLen, Inputs[i]->InputName.ToString().Len());
            }
            for (int32 i = 0; i < Outputs.Num(); i++)
            {
                maxOutputLen = FMath::Max(maxOutputLen, Outputs[i].OutputName.ToString().Len());
            }
            check(0 < maxOutputLen);
            CharCount = maxInputLen + maxOutputLen;
        }
        else if (Node.Expression->IsA<UMaterialExpressionTextureObjectParameter>())
        {
            RowCount = 5; // approximate height of a TextureObjectParameter
        }

        Node.Size.X = FMath::Max(CaptionWidth * CharWidth + BaseWidth, CharCount * CharWidth + (!!Inputs.Num() + !!Outputs.Num()) * BaseWidth);
        Node.Size.Y = (Caption.Num() + RowCount) * CHAR_HEIGHT;
    }
}

static void AdjustLevel(FNodeData* ParentNode, FNodeData* ChildNode, int32 Index, TMap<UMaterialExpression*, FNodeData>& Nodes)
{
    if (ChildNode->Level <= ParentNode->Level)
    {
        if (ChildNode->Anchor != ParentNode->Expression)
        {
            // this expression was anchored to a lower level -> remove it from that anchor
            FNodeData& AnchorNode = Nodes.FindChecked(ChildNode->Anchor);
            check(AnchorNode.Children.ContainsByPredicate([ChildNode](const FChildData& Child) { return Child.Expression == ChildNode->Expression; }));
            AnchorNode.Children.RemoveAll([ChildNode](const FChildData& Child) { return Child.Expression == ChildNode->Expression; });

            // ... and anchor it here
            ChildNode->Anchor = ParentNode->Expression;
        }
        ChildNode->Level = ParentNode->Level + 1;

        check(!ParentNode->Children.ContainsByPredicate([ChildNode](const FChildData& Child) { return Child.Expression == ChildNode->Expression; }));
        ParentNode->Children.Add({ ChildNode->Expression, Index });

        // adjust level of all inputs, recreating the set of children anchored to this node
        ChildNode->Children.Empty();
        for (int32 I = 0; I < ChildNode->Expression->GetInputs().Num(); I++)
        {
            UMaterialExpression* GrandChildExpression = ChildNode->Expression->GetInput(I)->Expression;
            if (GrandChildExpression)
            {
                FNodeData* GrandChildNode = Nodes.Find(GrandChildExpression);
                AdjustLevel(ChildNode, GrandChildNode, I, Nodes);
            }
        }
    }
}

static void DetermineHierarchy(const TArray<UMaterialExpression*>& Expressions, const UMaterialExpression* ParentExpression, TMap<UMaterialExpression*, FNodeData>& Nodes)
{
    FNodeData* ParentNode = Nodes.Find(ParentExpression);
    for (int32 I=0 ; I<Expressions.Num() ; I++)
    {
        UMaterialExpression* Expression = Expressions[I];
        if (Expression && !ParentNode->Children.ContainsByPredicate([Expression](const FChildData& Child) { return Child.Expression == Expression; }))
        {
            FNodeData* ChildNode = Nodes.Find(Expression);
            if (ChildNode == nullptr)
            {
                Nodes.Add(Expression, { Expression, ParentNode->Expression, ParentNode->Level + 1 });

                TArray<FExpressionInput*> const& Inputs = Expression->GetInputs();
                TArray<UMaterialExpression*> InputExpressions;
                InputExpressions.Reserve(Inputs.Num());
                for (FExpressionInput* Input : Inputs)
                {
                    InputExpressions.Add(Input->Expression);
                }
                DetermineHierarchy(InputExpressions, Expression, Nodes);

                ParentNode = Nodes.Find(ParentExpression);		// get the ParentNode again, as it might have been moved by the operations above
                check(!ParentNode->Children.ContainsByPredicate([Expression](const FChildData& Child) { return Child.Expression == Expression; }));
                ParentNode->Children.Add({ Expression, I });
            }
            else
            {
                AdjustLevel(ParentNode, ChildNode, I, Nodes);
            }
        }
    }
}

static int32 DetermineDelta(FInt2 const& BottomOffset, FInt2 const& BottomSize, FInt2 const& TopOffset, FInt2 const& TopSize)
{
    // if the bottom part is in the same x-range as the top part, calculate the delta to move the bottom part out of the top part range
    return ((BottomOffset.X + BottomSize.X + DELTA_WIDTH < TopOffset.X) || (TopOffset.X + TopSize.X + DELTA_WIDTH < BottomOffset.X)) ? 0 : TopOffset.Y + TopSize.Y + DELTA_HEIGHT - BottomOffset.Y;
}

static void DetermineRelativePositions(FNodeData* ParentNode, TMap<UMaterialExpression*, FNodeData>& Nodes)
{
    TArray<FNodeData*> ChildrenNodes;
    for (const FChildData& Child : ParentNode->Children)
    {
        check(Child.Expression && Nodes.Contains(Child.Expression));
        FNodeData* ChildNode = Nodes.Find(Child.Expression);

        // preliminary relative position of the child corresponding to its input into the parent and the output from the child
        ChildNode->Offset.X = -ChildNode->Size.X - DELTA_WIDTH;
        ChildNode->Offset.Y = (Child.InputIndex - (ParentNode->Expression ? ParentNode->Expression->GetInput(Child.InputIndex)->OutputIndex : 0)) * CHAR_HEIGHT;

        DetermineRelativePositions(ChildNode, Nodes);

        ChildrenNodes.Add(ChildNode);
    }

    // here, the relative positions of the grandchildren are set -> adjust the children's relative position to prevent intersections
    int32 DeltaSum = 0;
    for (int32 I = 1; I < ChildrenNodes.Num(); I++)
    {
        TArray<FNodeData*> TopNodeStack;
        TArray<FInt2> TopOffsetStack;
        for (int32 J = 0; J < I; J++)
        {
            TopNodeStack.Push(ChildrenNodes[J]);
            TopOffsetStack.Push(TopNodeStack.Last()->Offset);
        }
        FNodeData* BottomChildNode = ChildrenNodes[I];

        while (TopNodeStack.Num() != 0)
        {
            FNodeData* TopNode = TopNodeStack.Pop();
            FInt2 TopOffset = TopOffsetStack.Pop();
            // first check the bounding boxe of the bottom node against the bounding box of the top node
            int32 Delta = DetermineDelta(BottomChildNode->Offset + BottomChildNode->BBoxOffset, BottomChildNode->BBoxSize, TopOffset + TopNode->BBoxOffset, TopNode->BBoxSize);
            if (0 < Delta)
            {
                // the bounding boxes intersect -> stack the children of the top node for further testing
                if (TopNode->Children.Num() != 0)
                {
                    for (const FChildData& GrandChild : TopNode->Children)
                    {
                        TopNodeStack.Push(Nodes.Find(GrandChild.Expression));
                        TopOffsetStack.Push(TopNodeStack.Last()->Offset + TopOffset);
                    }
                }

                // check the bounding box of the bottom node against the simple box of the top node
                Delta = DetermineDelta(BottomChildNode->Offset + BottomChildNode->BBoxOffset, BottomChildNode->BBoxSize, TopOffset, TopNode->Size);
                if (0 < Delta)
                {
                    // they still intersect -> start a stack with the bottom node
                    TArray<FNodeData*> BottomNodeStack;
                    TArray<FInt2> BottomOffsetStack;
                    BottomNodeStack.Push(BottomChildNode);
                    BottomOffsetStack.Push(BottomChildNode->Offset);

                    while (BottomNodeStack.Num() != 0)
                    {
                        FNodeData* BottomNode = BottomNodeStack.Pop();
                        FInt2 BottomOffset = BottomOffsetStack.Pop();
                        // check the bounding box of the bottom node against the simple box of the top node
                        Delta = DetermineDelta(BottomOffset + BottomNode->BBoxOffset, BottomNode->BBoxSize, TopOffset, TopNode->Size);
                        if (0 < Delta)
                        {
                            // they still intersect -> stack the children of the bottom node for further testing
                            if (BottomNode->Children.Num() != 0)
                            {
                                for (const FChildData& GrandChild : BottomNode->Children)
                                {
                                    BottomNodeStack.Push(Nodes.Find(GrandChild.Expression));
                                    BottomOffsetStack.Push(BottomNodeStack.Last()->Offset + BottomOffset);
                                }
                            }

                            // and check the simple box of the bottom node against the simple box of the top node
                            Delta = DetermineDelta(BottomOffset, BottomNode->Size, TopOffset, TopNode->Size);
                            if (0 < Delta)
                            {
                                // they still instersect -> move all bottom nodes down accordingly
                                for (int32 J = I; J < ChildrenNodes.Num(); J++)
                                {
                                    ChildrenNodes[J]->Offset.Y += Delta;
                                }
                                // and adjust the offsets of the already stacked bottom nodes
                                for (FInt2& Offset : BottomOffsetStack)
                                {
                                    Offset.Y += Delta;
                                }
                                DeltaSum += Delta;
                            }
                        }
                    }
                }
            }
        }
    }

    // if some nodes have been moved down, move everything have that amount up again
    if (0 < DeltaSum)
    {
        int32 Shift = DeltaSum / 2;
        for (FNodeData* ChildNode : ChildrenNodes)
        {
            ChildNode->Offset.Y -= Shift;
        }
    }

    // now that all children are fixed, determine the bounding box of the parent node
    ParentNode->BBoxOffset = { 0, 0 };
    ParentNode->BBoxSize = ParentNode->Size;
    for (FNodeData* ChildNode : ChildrenNodes)
    {
        // determine the offset of the childrens bounding box relative to the parent node
        // and adjust the parent node's bounding box offset and bounding box size if that point is out of its current bounding box
        FInt2 Offset = ChildNode->Offset + ChildNode->BBoxOffset;
        FInt2 Delta = ParentNode->BBoxOffset - Offset;
        if (0 < Delta.X)
        {
            ParentNode->BBoxOffset.X = Offset.X;
            ParentNode->BBoxSize.X += Delta.X;
        }
        if (0 < Delta.Y)
        {
            ParentNode->BBoxOffset.Y = Offset.Y;
            ParentNode->BBoxSize.Y += Delta.Y;
        }

        // determine the delta between the childrens lower left bounding box corner and the parents lower left bounding box corner
        // and adjust the parent node's bounding box size if that delta is positive
        Delta = Offset + ChildNode->BBoxSize - (ParentNode->BBoxOffset + ParentNode->BBoxSize);
        if (0 < Delta.X)
        {
            ParentNode->BBoxSize.X += Delta.X;
        }
        if (0 < Delta.Y)
        {
            ParentNode->BBoxSize.Y += Delta.Y;
        }
    }
}

static void SetNodeEditorPositions(const FNodeData* Node, TMap<UMaterialExpression*, FNodeData>& Nodes)
{
    if (Node->Expression)
    {
        FInt2 Pos = Node->Offset;
        if (Node->Anchor)
        {
            FNodeData* ParentNode = Nodes.Find(Node->Anchor);
            Pos.X += ParentNode->Expression->MaterialExpressionEditorX;
            Pos.Y += ParentNode->Expression->MaterialExpressionEditorY;
        }
        Node->Expression->MaterialExpressionEditorX = Pos.X;
        Node->Expression->MaterialExpressionEditorY = Pos.Y;
    }

    for (const FChildData& Child : Node->Children)
    {
        SetNodeEditorPositions(Nodes.Find(Child.Expression), Nodes);
    }
}

void ArrangeNodes(UObject* Parent, const TArray<UMaterialExpression*>& OutputExpressions)
{
    check(Parent->IsA<UMaterial>() || Parent->IsA<UMaterialFunction>());

    TMap<UMaterialExpression*, FNodeData> Nodes;
    Nodes.Add(nullptr);
    DetermineHierarchy(OutputExpressions, nullptr, Nodes);
    FNodeData* RootNode = Nodes.Find(nullptr);

    if (Parent->IsA<UMaterial>())
    {
        // remove any unreferenced expressions
        TArray<UMaterialExpression*> Orphans;
        for (UMaterialExpression* Expression : Cast<UMaterial>(Parent)->Expressions)
        {
            if (!Nodes.Contains(Expression))
            {
                Orphans.Add(Expression);
            }
        }
        for (UMaterialExpression* Orphan : Orphans)
        {
            Cast<UMaterial>(Parent)->Expressions.Remove(Orphan);
        }
    }
    else
    {
        // add any unreferenced expressions to the RootNode
        for (UMaterialExpression* Expression : Cast<UMaterialFunction>(Parent)->FunctionExpressions)
        {
            if (!Nodes.Contains(Expression))
            {
                DetermineHierarchy({ Expression }, nullptr, Nodes);
            }
        }
    }

    for (auto& NodeIt : Nodes)
    {
        EstimateSize(NodeIt.Value);
    }
    RootNode = Nodes.Find(nullptr);		// get the RootNode again, as it might have been moved by the operations above
    RootNode->Offset.X = 0;
    RootNode->Offset.Y = 0;
    DetermineRelativePositions(RootNode, Nodes);
    SetNodeEditorPositions(RootNode, Nodes);
}

#endif