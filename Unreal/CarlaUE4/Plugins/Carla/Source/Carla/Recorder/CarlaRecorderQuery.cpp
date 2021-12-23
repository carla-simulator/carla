// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderHelpers.h"

#include <ctime>
#include <sstream>
#include <string>

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/VehicleLightState.h>
#include <carla/rpc/VehiclePhysicsControl.h>
#include <compiler/enable-ue4-macros.h>

inline bool CarlaRecorderQuery::ReadHeader(void)
{
  if (File.eof())
  {
    return false;
  }

  ReadValue<char>(File, Header.Id);
  ReadValue<uint32_t>(File, Header.Size);

  return true;
}

inline void CarlaRecorderQuery::SkipPacket(void)
{
  File.seekg(Header.Size, std::ios::cur);
}

inline bool CarlaRecorderQuery::CheckFileInfo(std::stringstream &Info)
{
  // read Info
  RecInfo.Read(File);

  // check magic string
  if (RecInfo.Magic != "CARLA_RECORDER")
  {
    Info << "File is not a CARLA recorder" << std::endl;
    return false;
  }

  // show general Info
  Info << "Version: " << RecInfo.Version << std::endl;
  Info << "Map: " << TCHAR_TO_UTF8(*RecInfo.Mapfile) << std::endl;
  tm *TimeInfo = localtime(&RecInfo.Date);
  char DateStr[100];
  strftime(DateStr, sizeof(DateStr), "%x %X", TimeInfo);
  Info << "Date: " << DateStr << std::endl << std::endl;

  return true;
}

std::string CarlaRecorderQuery::QueryInfo(std::string Filename, bool bShowAll)
{
  std::stringstream Info;

  // get the final path + filename
  std::string Filename2 = GetRecorderFilename(Filename);

  // try to open
  File.open(Filename2, std::ios::binary);
  if (!File.is_open())
  {
    Info << "File " << Filename2 << " not found on server\n";
    return Info.str();
  }

  uint16_t i, Total;
  bool bFramePrinted = false;

  // lambda for repeating task
  auto PrintFrame = [this](std::stringstream &Info)
  {
    Info << "Frame " << Frame.Id << " at " << Frame.Elapsed << " seconds\n";
  };

  if (!CheckFileInfo(Info))
    return Info.str();

  // parse only frames
  while (File)
  {
    // get header
    if (!ReadHeader())
    {
      break;
    }

    // check for a frame packet
    switch (Header.Id)
    {
      // frame
      case static_cast<char>(CarlaRecorderPacketId::FrameStart):
        Frame.Read(File);
        if (bShowAll)
        {
          PrintFrame(Info);
          bFramePrinted = true;
        }
        else
          bFramePrinted = false;
        break;

      // events add
      case static_cast<char>(CarlaRecorderPacketId::EventAdd):
        ReadValue<uint16_t>(File, Total);
        if (Total > 0 && !bFramePrinted)
        {
          PrintFrame(Info);
          bFramePrinted = true;
        }
        for (i = 0; i < Total; ++i)
        {
          // add
          EventAdd.Read(File);
          Info << " Create " << EventAdd.DatabaseId << ": " << TCHAR_TO_UTF8(*EventAdd.Description.Id) <<
            " (" <<
            static_cast<int>(EventAdd.Type) << ") at (" << EventAdd.Location.X << ", " <<
            EventAdd.Location.Y << ", " << EventAdd.Location.Z << ")" << std::endl;
          for (auto &Att : EventAdd.Description.Attributes)
          {
            Info << "  " << TCHAR_TO_UTF8(*Att.Id) << " = " << TCHAR_TO_UTF8(*Att.Value) << std::endl;
          }
        }
        break;

      // events del
      case static_cast<char>(CarlaRecorderPacketId::EventDel):
        ReadValue<uint16_t>(File, Total);
        if (Total > 0 && !bFramePrinted)
        {
          PrintFrame(Info);
          bFramePrinted = true;
        }
        for (i = 0; i < Total; ++i)
        {
          EventDel.Read(File);
          Info << " Destroy " << EventDel.DatabaseId << "\n";
        }
        break;

      // events parenting
      case static_cast<char>(CarlaRecorderPacketId::EventParent):
        ReadValue<uint16_t>(File, Total);
        if (Total > 0 && !bFramePrinted)
        {
          PrintFrame(Info);
          bFramePrinted = true;
        }
        for (i = 0; i < Total; ++i)
        {
          EventParent.Read(File);
          Info << " Parenting " << EventParent.DatabaseId << " with " << EventParent.DatabaseIdParent <<
            " (parent)\n";
        }
        break;

      // collisions
      case static_cast<char>(CarlaRecorderPacketId::Collision):
        ReadValue<uint16_t>(File, Total);
        if (Total > 0 && !bFramePrinted)
        {
          PrintFrame(Info);
          bFramePrinted = true;
        }
        for (i = 0; i < Total; ++i)
        {
          Collision.Read(File);
          Info << " Collision id " << Collision.Id << " between " << Collision.DatabaseId1;
          if (Collision.IsActor1Hero)
            Info << " (hero) ";
          Info << " with " << Collision.DatabaseId2;
          if (Collision.IsActor2Hero)
            Info << " (hero) ";
          Info << std::endl;
        }
        break;

      // positions
      case static_cast<char>(CarlaRecorderPacketId::Position):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Positions: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            Position.Read(File);
            Info << "  Id: " << Position.DatabaseId << " Location: (" << Position.Location.X << ", " << Position.Location.Y << ", " << Position.Location.Z << ") Rotation (" <<  Position.Rotation.X << ", " << Position.Rotation.Y << ", " << Position.Rotation.Z << ")" << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      // traffic light
      case static_cast<char>(CarlaRecorderPacketId::State):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " State traffic lights: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            StateTraffic.Read(File);
            Info << "  Id: " << StateTraffic.DatabaseId << " state: " << static_cast<char>(0x30 + StateTraffic.State) << " frozen: " <<
              StateTraffic.IsFrozen << " elapsedTime: " << StateTraffic.ElapsedTime << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      // vehicle animations
      case static_cast<char>(CarlaRecorderPacketId::AnimVehicle):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Vehicle animations: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            Vehicle.Read(File);
            Info << "  Id: " << Vehicle.DatabaseId << " Steering: " << Vehicle.Steering << " Throttle: " << Vehicle.Throttle << " Brake " << Vehicle.Brake << " Handbrake: " << Vehicle.bHandbrake << " Gear: " << Vehicle.Gear << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      // walker animations
      case static_cast<char>(CarlaRecorderPacketId::AnimWalker):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Walker animations: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            Walker.Read(File);
            Info << "  Id: " << Walker.DatabaseId << " speed: " << Walker.Speed << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      // vehicle light animations
      case static_cast<char>(CarlaRecorderPacketId::VehicleLight):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Vehicle light animations: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            LightVehicle.Read(File);

            carla::rpc::VehicleLightState LightState(LightVehicle.State);
            FVehicleLightState State(LightState);
            std::string enabled_lights_list;
            if (State.Position)
              enabled_lights_list += "Position ";
            if (State.LowBeam)
              enabled_lights_list += "LowBeam ";
            if (State.HighBeam)
              enabled_lights_list += "HighBeam ";
            if (State.Brake)
              enabled_lights_list += "Brake ";
            if (State.RightBlinker)
              enabled_lights_list += "RightBlinker ";
            if (State.LeftBlinker)
              enabled_lights_list += "LeftBlinker ";
            if (State.Reverse)
              enabled_lights_list += "Reverse ";
            if (State.Interior)
              enabled_lights_list += "Interior ";
            if (State.Fog)
              enabled_lights_list += "Fog ";
            if (State.Special1)
              enabled_lights_list += "Special1 ";
            if (State.Special2)
              enabled_lights_list += "Special2 ";

            if (enabled_lights_list.size())
            {
              Info << "  Id: " << LightVehicle.DatabaseId << " " <<
                  enabled_lights_list.substr(0, enabled_lights_list.size() - 1) << std::endl;
            }
            else
            {
              Info << "  Id: " << LightVehicle.DatabaseId << " None" << std::endl;
            }
          }
        }
        else
          SkipPacket();
        break;

      // scene light animations
      case static_cast<char>(CarlaRecorderPacketId::SceneLight):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Scene light changes: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            LightScene.Read(File);
            Info << "  Id: " << LightScene.LightId << " enabled: " << (LightScene.bOn ? "True" : "False")
                << " intensity: " << LightScene.Intensity
                << " RGB_color: (" << LightScene.Color.R << ", " << LightScene.Color.G << ", " << LightScene.Color.B << ")"
                << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      // dynamic actor kinematics
      case static_cast<char>(CarlaRecorderPacketId::Kinematics):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Dynamic actors: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            Kinematics.Read(File);
            Info << "  Id: " << Kinematics.DatabaseId << " linear_velocity: ("
                << Kinematics.LinearVelocity.X << ", " << Kinematics.LinearVelocity.Y << ", " << Kinematics.LinearVelocity.Z << ")"
                << " angular_velocity: ("
                << Kinematics.AngularVelocity.X << ", " << Kinematics.AngularVelocity.Y << ", " << Kinematics.AngularVelocity.Z << ")"
                << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      // actors bounding boxes
      case static_cast<char>(CarlaRecorderPacketId::BoundingBox):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Actor bounding boxes: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            ActorBoundingBox.Read(File);
            Info << "  Id: " << ActorBoundingBox.DatabaseId << " origin: ("
                << ActorBoundingBox.BoundingBox.Origin.X << ", "
                << ActorBoundingBox.BoundingBox.Origin.Y << ", "
                << ActorBoundingBox.BoundingBox.Origin.Z << ")"
                << " extension: ("
                << ActorBoundingBox.BoundingBox.Extension.X << ", "
                << ActorBoundingBox.BoundingBox.Extension.Y << ", "
                << ActorBoundingBox.BoundingBox.Extension.Z << ")"
                << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      // actors trigger volumes
      case static_cast<char>(CarlaRecorderPacketId::TriggerVolume):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Actor trigger volumes: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            ActorBoundingBox.Read(File);
            Info << "  Id: " << ActorBoundingBox.DatabaseId << " origin: ("
                << ActorBoundingBox.BoundingBox.Origin.X << ", "
                << ActorBoundingBox.BoundingBox.Origin.Y << ", "
                << ActorBoundingBox.BoundingBox.Origin.Z << ")"
                << " extension: ("
                << ActorBoundingBox.BoundingBox.Extension.X << ", "
                << ActorBoundingBox.BoundingBox.Extension.Y << ", "
                << ActorBoundingBox.BoundingBox.Extension.Z << ")"
                << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      // Platform time
      case static_cast<char>(CarlaRecorderPacketId::PlatformTime):
        if (bShowAll)
        {
          if (!bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }

          PlatformTime.Read(File);
          Info << " Current platform time: " << PlatformTime.Time << std::endl;
        }
        else
          SkipPacket();
        break;

      case static_cast<char>(CarlaRecorderPacketId::PhysicsControl):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }

          Info << " Physics Control events: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            PhysicsControl.Read(File);
            carla::rpc::VehiclePhysicsControl Control(PhysicsControl.VehiclePhysicsControl);
            Info << "  Id: " << PhysicsControl.DatabaseId << std::endl
                << "   max_rpm = " << Control.max_rpm << std::endl
                << "   MOI = " << Control.moi << std::endl
                << "   damping_rate_full_throttle = " << Control.damping_rate_full_throttle << std::endl
                << "   damping_rate_zero_throttle_clutch_engaged = " << Control.damping_rate_zero_throttle_clutch_engaged << std::endl
                << "   damping_rate_zero_throttle_clutch_disengaged = " << Control.damping_rate_zero_throttle_clutch_disengaged << std::endl
                << "   use_gear_auto_box = " << (Control.use_gear_autobox ? "true" : "false") << std::endl
                << "   gear_switch_time = " << Control.gear_switch_time << std::endl
                << "   clutch_strength = " << Control.clutch_strength << std::endl
                << "   final_ratio = " << Control.final_ratio << std::endl
                << "   mass = " << Control.mass << std::endl
                << "   drag_coefficient = " << Control.drag_coefficient << std::endl
                << "   center_of_mass = " << "(" << Control.center_of_mass.x << ", " << Control.center_of_mass.y << ", " << Control.center_of_mass.z << ")" << std::endl;
            Info << "   torque_curve =";
            for (auto& vec : Control.torque_curve)
            {
              Info << " (" << vec.x << ", " << vec.y << ")";
            }
            Info << std::endl;
            Info << "   steering_curve =";
            for (auto& vec : Control.steering_curve)
            {
              Info << " (" << vec.x << ", " << vec.y << ")";
            }
            Info << std::endl;
            Info << "   forward_gears:" << std::endl;
            uint32_t count = 0;
            for (auto& Gear : Control.forward_gears)
            {
              Info << "    gear " << count << ": ratio " << Gear.ratio
                  << " down_ratio " << Gear.down_ratio
                  << " up_ratio " << Gear.up_ratio << std::endl;
              ++count;
            }
            Info << "   wheels:" << std::endl;
            count = 0;
            for (auto& Wheel : Control.wheels)
            {
              Info << "    wheel " << count << ": tire_friction " << Wheel.tire_friction
                  << " damping_rate " << Wheel.damping_rate
                  << " max_steer_angle " << Wheel.max_steer_angle
                  << " radius " << Wheel.radius
                  << " max_brake_torque " << Wheel.max_brake_torque
                  << " max_handbrake_torque " << Wheel.max_handbrake_torque
                  << " position " << "(" << Wheel.position.x << ", " << Wheel.position.y << ", " << Wheel.position.z << ")"
                  << std::endl;
              ++count;
            }
          }
        }
        else
          SkipPacket();
        break;

      case static_cast<char>(CarlaRecorderPacketId::TrafficLightTime):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }

          Info << " Traffic Light time events: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            TrafficLightTime.Read(File);
            Info << "  Id: " << TrafficLightTime.DatabaseId
                << " green_time: " << TrafficLightTime.GreenTime
                << " yellow_time: " << TrafficLightTime.YellowTime
                << " red_time: " << TrafficLightTime.RedTime
                << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      case static_cast<char>(CarlaRecorderPacketId::WalkerBones):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }

          Info << " Walkers Bones: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            WalkerBones.Clear();
            WalkerBones.Read(File);
            Info << "  Id: " << WalkerBones.DatabaseId << "\n";
            for (const auto &Bone : WalkerBones.Bones)
            {
              Info << "     Bone: \"" << TCHAR_TO_UTF8(*Bone.Name) << "\" relative: " << "Loc("
                   << Bone.Location.X << ", " << Bone.Location.Y << ", " << Bone.Location.Z << ") Rot(" 
                   << Bone.Rotation.X << ", " << Bone.Rotation.Y << ", " << Bone.Rotation.Z << ")\n";
            }
          }
          Info << std::endl;
        }
        else
          SkipPacket();
        break;

        // frame end
      case static_cast<char>(CarlaRecorderPacketId::FrameEnd):
        // do nothing, it is empty
        break;

      default:
        SkipPacket();
        break;
    }
  }

  Info << "\nFrames: " << Frame.Id << "\n";
  Info << "Duration: " << Frame.Elapsed << " seconds\n";

  File.close();

  return Info.str();
}

std::string CarlaRecorderQuery::QueryCollisions(std::string Filename, char Category1, char Category2)
{
  std::stringstream Info;

  // get the final path + filename
  std::string Filename2 = GetRecorderFilename(Filename);

  // try to open
  File.open(Filename2, std::ios::binary);
  if (!File.is_open())
  {
    Info << "File " << Filename2 << " not found on server\n";
    return Info.str();
  }

  if (!CheckFileInfo(Info))
    return Info.str();

  // other, vehicle, walkers, trafficLight, hero, any
  char Categories[] = { 'o', 'v', 'w', 't', 'h', 'a' };
  uint16_t i, Total;
  struct ReplayerActorInfo
  {
    uint8_t Type;
    FString Id;
  };
  std::unordered_map<uint32_t, ReplayerActorInfo> Actors;
  struct PairHash
  {
    std::size_t operator()(const std::pair<uint32_t, uint32_t>& P) const
    {
        std::size_t hash = P.first;
        hash <<= 32;
        hash += P.second;
        return hash;
    }
  };
  std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash > oldCollisions, newCollisions;

  // header
  Info << std::setw(8) << "Time";
  Info << " " << std::setw(6) << "Types";
  Info << " " << std::setw(6) << std::right << "Id";
  Info << " " << std::setw(35) << std::left << "Actor 1";
  Info << " " << std::setw(6) << std::right << "Id";
  Info << " " << std::setw(35) << std::left << "Actor 2";
  Info << std::endl;

  // parse only frames
  while (File)
  {

    // get header
    if (!ReadHeader())
    {
      break;
    }

    // check for a frame packet
    switch (Header.Id)
    {
      // frame
      case static_cast<char>(CarlaRecorderPacketId::FrameStart):
        Frame.Read(File);
        // exchange sets of collisions (to know when a collision is new or continue from previous frame)
        oldCollisions = std::move(newCollisions);
        newCollisions.clear();
        break;

      // events add
      case static_cast<char>(CarlaRecorderPacketId::EventAdd):
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          // add
          EventAdd.Read(File);
          Actors[EventAdd.DatabaseId] = ReplayerActorInfo { EventAdd.Type, EventAdd.Description.Id };
        }
        break;

      // events del
      case static_cast<char>(CarlaRecorderPacketId::EventDel):
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          EventDel.Read(File);
          Actors.erase(EventAdd.DatabaseId);
        }
        break;

      // events parenting
      case static_cast<char>(CarlaRecorderPacketId::EventParent):
        SkipPacket();
        break;

      // collisions
      case static_cast<char>(CarlaRecorderPacketId::Collision):
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          Collision.Read(File);

          int Valid = 0;

          // get categories for both actors
          uint8_t Type1, Type2;
          if (Collision.DatabaseId1 != uint32_t(-1))
            Type1 = Categories[Actors[Collision.DatabaseId1].Type];
          else
            Type1 = 'o'; // other non-actor object
          
          if (Collision.DatabaseId2 != uint32_t(-1))
            Type2 = Categories[Actors[Collision.DatabaseId2].Type];
          else
            Type2 = 'o'; // other non-actor object

          // filter actor 1
          if (Category1 == 'a')
            ++Valid;
          else if (Category1 == Type1)
            ++Valid;
          else if (Category1 == 'h' && Collision.IsActor1Hero)
            ++Valid;

          // filter actor 2
          if (Category2 == 'a')
            ++Valid;
          else if (Category2 == Type2)
            ++Valid;
          else if (Category2 == 'h' && Collision.IsActor2Hero)
            ++Valid;

          // only show if both actors has passed the filter
          if (Valid == 2)
          {
            // check if we need to show as a starting collision or it is a continuation one
            auto collisionPair = std::make_pair(Collision.DatabaseId1, Collision.DatabaseId2);
            if (oldCollisions.count(collisionPair) == 0)
            {
              Info << std::setw(8) << std::setprecision(0) << std::right << std::fixed << Frame.Elapsed;
              Info << " " << "  " << Type1 << " " << Type2 << " ";
              Info << " " << std::setw(6) << std::right << Collision.DatabaseId1;
              Info << " " << std::setw(35) << std::left << TCHAR_TO_UTF8(*Actors[Collision.DatabaseId1].Id);
              Info << " " << std::setw(6) << std::right << Collision.DatabaseId2;
              Info << " " << std::setw(35) << std::left << TCHAR_TO_UTF8(*Actors[Collision.DatabaseId2].Id);
              Info << std::endl;
            }
            // save current collision
            newCollisions.insert(collisionPair);
          }
        }
        break;

      case static_cast<char>(CarlaRecorderPacketId::Position):
        SkipPacket();
        break;

      case static_cast<char>(CarlaRecorderPacketId::State):
        SkipPacket();
        break;

      // frame end
      case static_cast<char>(CarlaRecorderPacketId::FrameEnd):
        // do nothing, it is empty
        break;

      default:
        SkipPacket();
        break;
    }
  }

  Info << "\nFrames: " << Frame.Id << "\n";
  Info << "Duration: " << Frame.Elapsed << " seconds\n";

  File.close();

  return Info.str();
}

std::string CarlaRecorderQuery::QueryBlocked(std::string Filename, double MinTime, double MinDistance)
{
  std::stringstream Info;

  // get the final path + filename
  std::string Filename2 = GetRecorderFilename(Filename);

  // try to open
  File.open(Filename2, std::ios::binary);
  if (!File.is_open())
  {
    Info << "File " << Filename2 << " not found on server\n";
    return Info.str();
  }

  if (!CheckFileInfo(Info))
    return Info.str();

  // other, vehicle, walkers, trafficLight, hero, any
  uint16_t i, Total;
  struct ReplayerActorInfo
  {
    uint8_t Type;
    FString Id;
    FVector LastPosition;
    double Time;
    double Duration;
  };
  std::unordered_map<uint32_t, ReplayerActorInfo> Actors;
  // to be able to sort the results by the duration of each actor (decreasing order)
  std::multimap<double, std::string, std::greater<double>> Results;

  // header
  Info << std::setw(8) << "Time";
  Info << " " << std::setw(6) << "Id";
  Info << " " << std::setw(35) << std::left << "Actor";
  Info << " " << std::setw(10) << std::right << "Duration";
  Info << std::endl;

  // parse only frames
  while (File)
  {

    // get header
    if (!ReadHeader())
    {
      break;
    }

    // check for a frame packet
    switch (Header.Id)
    {
      // frame
      case static_cast<char>(CarlaRecorderPacketId::FrameStart):
        Frame.Read(File);
        break;

      // events add
      case static_cast<char>(CarlaRecorderPacketId::EventAdd):
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          // add
          EventAdd.Read(File);
          Actors[EventAdd.DatabaseId] = ReplayerActorInfo { EventAdd.Type, EventAdd.Description.Id, FVector(0, 0, 0), 0.0, 0.0 };
        }
        break;

      // events del
      case static_cast<char>(CarlaRecorderPacketId::EventDel):
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          EventDel.Read(File);
          Actors.erase(EventAdd.DatabaseId);
        }
        break;

      // events parenting
      case static_cast<char>(CarlaRecorderPacketId::EventParent):
        SkipPacket();
        break;

      // collisions
      case static_cast<char>(CarlaRecorderPacketId::Collision):
        SkipPacket();
        break;

      // positions
      case static_cast<char>(CarlaRecorderPacketId::Position):
        // read all positions
        ReadValue<uint16_t>(File, Total);
        for (i=0; i<Total; ++i)
        {
          Position.Read(File);
          // check if actor moved less than a distance
          if (FVector::Distance(Actors[Position.DatabaseId].LastPosition, Position.Location) < MinDistance)
          {
            // actor stopped
            if (Actors[Position.DatabaseId].Duration == 0)
              Actors[Position.DatabaseId].Time = Frame.Elapsed;
            Actors[Position.DatabaseId].Duration += Frame.DurationThis;
          }
          else
          {
            // check to show info
            if (Actors[Position.DatabaseId].Duration >= MinTime)
            {
              std::stringstream Result;
              Result << std::setw(8) << std::setprecision(0) << std::fixed << Actors[Position.DatabaseId].Time;
              Result << " " << std::setw(6) << Position.DatabaseId;
              Result << " " << std::setw(35) << std::left << TCHAR_TO_UTF8(*Actors[Position.DatabaseId].Id);
              Result << " " << std::setw(10) << std::setprecision(0) << std::fixed << std::right << Actors[Position.DatabaseId].Duration;
              Result << std::endl;
              Results.insert(std::make_pair(Actors[Position.DatabaseId].Duration, Result.str()));
            }
            // actor moving
            Actors[Position.DatabaseId].Duration = 0;
            Actors[Position.DatabaseId].LastPosition = Position.Location;
          }
        }
        break;

      // traffic light
      case static_cast<char>(CarlaRecorderPacketId::State):
        SkipPacket();
        break;

      // frame end
      case static_cast<char>(CarlaRecorderPacketId::FrameEnd):
        // do nothing, it is empty
        break;

      default:
        SkipPacket();
        break;
    }
  }

  // show actors stopped that were not moving again
  for (auto &Actor : Actors)
  {
    // check to show info
    if (Actor.second.Duration >= MinTime)
    {
      std::stringstream Result;
      Result << std::setw(8) << std::setprecision(0) << std::fixed << Actor.second.Time;
      Result << " " << std::setw(6) << Actor.first;
      Result << " " << std::setw(35) << std::left << TCHAR_TO_UTF8(*Actor.second.Id);
      Result << " " << std::setw(10) << std::setprecision(0) << std::fixed << std::right << Actor.second.Duration;
      Result << std::endl;
      Results.insert(std::make_pair(Actor.second.Duration, Result.str()));
    }
  }

  // show the result
  for (auto &Result : Results)
  {
    Info << Result.second;
  }

  Info << "\nFrames: " << Frame.Id << "\n";
  Info << "Duration: " << Frame.Elapsed << " seconds\n";

  File.close();

  return Info.str();
}
