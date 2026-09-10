// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "HAL/Platform.h"
#include "UObject/ObjectMacros.h"

#include "TLLightType.generated.h"

UENUM(BlueprintType)
enum class ETLLightType : uint8
{
	SolidColorGreen UMETA(DisplayName = "Solid Color (Green)"),
	SolidColorAmber UMETA(DisplayName = "Solid Color (Amber)"),
	SolidColorRed UMETA(DisplayName = "Solid Color (Red)"),
	PedestrianWalkGreen UMETA(DisplayName = "Pedestrian Walk (Green)"),
	PedestrianWalkRed UMETA(DisplayName = "Pedestrian Walk (Red)"),
	PedestrianGreen UMETA(DisplayName = "Pedestrian (Green)"),
	PedestrianAmber UMETA(DisplayName = "Pedestrian (Amber)"),
	PedestrianRed UMETA(DisplayName = "Pedestrian (Red)"),
	BikeGreen UMETA(DisplayName = "Bike (Green)"),
	BikeAmber UMETA(DisplayName = "Bike (Amber)"),
	BikeRed UMETA(DisplayName = "Bike (Red)"),
	PedestrianWalkBikeGreen UMETA(DisplayName = "Pedestrian Walk Bike (Green)"),
	PedestrianWalkBikeRed UMETA(DisplayName = "Pedestrian Walk Bike (Red)"),
	PedestrianBikeGreen UMETA(DisplayName = "Pedestrian Bike (Green)"),
	PedestrianBikeRed UMETA(DisplayName = "Pedestrian Bike (Red)"),
	PedestrianStop UMETA(DisplayName = "Pedestrian Stop"),
	CrossGreen UMETA(DisplayName = "Cross (Green)"),
	CrossAmber UMETA(DisplayName = "Cross (Amber)"),
	CrossRed UMETA(DisplayName = "Cross (Red)"),
	ArrowLeftGreen UMETA(DisplayName = "Arrow Left (Green)"),
	ArrowLeftAmber UMETA(DisplayName = "Arrow Left (Amber)"),
	ArrowLeftRed UMETA(DisplayName = "Arrow Left (Red)"),
	ArrowRightGreen UMETA(DisplayName = "Arrow Right (Green)"),
	ArrowRightAmber UMETA(DisplayName = "Arrow Right (Amber)"),
	ArrowRightRed UMETA(DisplayName = "Arrow Right (Red)"),
	ArrowStraightGreen UMETA(DisplayName = "Arrow Straight (Green)"),
	ArrowStraightAmber UMETA(DisplayName = "Arrow Straight (Amber)"),
	ArrowStraightRed UMETA(DisplayName = "Arrow Straight (Red)"),
	ArrowDownGreen UMETA(DisplayName = "Arrow Down (Green)"),
	ArrowDownAmber UMETA(DisplayName = "Arrow Down (Amber)"),
	ArrowDownRed UMETA(DisplayName = "Arrow Down (Red)"),
	ArrowUpLeftGreen UMETA(DisplayName = "Arrow Up Left (Green)"),
	ArrowUpLeftAmber UMETA(DisplayName = "Arrow Up Left (Amber)"),
	ArrowUpLeftRed UMETA(DisplayName = "Arrow Up Left (Red)"),
	ArrowUpRightGreen UMETA(DisplayName = "Arrow Up Right (Green)"),
	ArrowUpRightAmber UMETA(DisplayName = "Arrow Up Right (Amber)"),
	ArrowUpRightRed UMETA(DisplayName = "Arrow Up Right (Red)"),
	ArrowDownRightGreen UMETA(DisplayName = "Arrow Down Right (Green)"),
	ArrowDownRightAmber UMETA(DisplayName = "Arrow Down Right (Amber)"),
	ArrowDownRightRed UMETA(DisplayName = "Arrow Down Right (Red)"),
	ArrowDownLeftGreen UMETA(DisplayName = "Arrow Down Left (Green)"),
	ArrowDownLeftAmber UMETA(DisplayName = "Arrow Down Left (Amber)"),
	ArrowDownLeftRed UMETA(DisplayName = "Arrow Down Left (Red)"),
	ForwardLeftGreen UMETA(DisplayName = "Forward Left (Green)"),
	ForwardLeftAmber UMETA(DisplayName = "Forward Left (Amber)"),
	ForwardLeftRed UMETA(DisplayName = "Forward Left (Red)"),
	ForwardRightGreen UMETA(DisplayName = "Forward Right (Green)"),
	ForwardRightAmber UMETA(DisplayName = "Forward Right (Amber)"),
	ForwardRightRed UMETA(DisplayName = "Forward Right (Red)"),
	RightTurnAndRightUTurnGreen UMETA(DisplayName = "Right Turn & Right U-Turn (Green)"),
	RightTurnAndRightUTurnAmber UMETA(DisplayName = "Right Turn & Right U-Turn (Amber)"),
	RightTurnAndRightUTurnRed UMETA(DisplayName = "Right Turn & Right U-Turn (Red)"),
	LeftTurnAndLeftUTurnGreen UMETA(DisplayName = "Left Turn & Left U-Turn (Green)"),
	LeftTurnAndLeftUTurnAmber UMETA(DisplayName = "Left Turn & Left U-Turn (Amber)"),
	LeftTurnAndLeftUTurnRed UMETA(DisplayName = "Left Turn & Left U-Turn (Red)"),
	RightUTurnGreen UMETA(DisplayName = "Right UTurn (Green)"),
	RightUTurnAmber UMETA(DisplayName = "Right UTurn (Amber)"),
	RightUTurnRed UMETA(DisplayName = "Right UTurn (Red)"),
	LeftUTurnGreen UMETA(DisplayName = "Left UTurn (Green)"),
	LeftUTurnAmber UMETA(DisplayName = "Left UTurn (Amber)"),
	LeftUTurnRed UMETA(DisplayName = "Left UTurn (Red)"),
	Tram UMETA(DisplayName = "Tram"),
	BusGreen UMETA(DisplayName = "Bus (Green)"),
	BusAmber UMETA(DisplayName = "Bus (Amber)"),
	BusRed UMETA(DisplayName = "Bus (Red)"),
	TramVerticalLine UMETA(DisplayName = "Tram Vertical Line"),
	TramHorizontalLine UMETA(DisplayName = "Tram Horizontal Line"),
	TramDiagonalLineForwardSlash UMETA(DisplayName = "Diagonal Line (/)"),
	TramDiagonalLineBackSlash UMETA(DisplayName = "Diagonal Line (\\)"),
	Circle UMETA(DisplayName = "Circle"),
	Triangle UMETA(DisplayName = "Triangle"),
};
