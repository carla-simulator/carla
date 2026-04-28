// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "vehicle_import/BPAutopicker.h"

#include <QString>
#include <cmath>

namespace carla_studio::vehicle_import {

namespace {
struct VehProto { float lengthCm; const char *path; };

constexpr VehProto kProtos[] = {
  {  230, "/Game/Carla/Blueprints/USDImportTemplates/BaseUSDImportVehicle" },
  {  230, "/Game/Carla/Blueprints/Vehicles/BmwIsetta/BP_BmwIsetta" },
  {  360, "/Game/Carla/Blueprints/Vehicles/NissanMicra/BP_NissanMicra" },
  {  370, "/Game/Carla/Blueprints/Vehicles/MiniCooper/BP_MiniCooper" },
  {  380, "/Game/Carla/Blueprints/Vehicles/AudiA2/BP_AudiA2" },
  {  395, "/Game/Carla/Blueprints/Vehicles/CitroenC3/BP_CitroenC3" },
  {  415, "/Game/Carla/Blueprints/Vehicles/AudiTT/BP_AudiTT" },
  {  420, "/Game/Carla/Blueprints/Vehicles/SeatLeon/BP_SeatLeon" },
  {  430, "/Game/Carla/Blueprints/Vehicles/VolkswagenT2/BP_VolkswagenT2" },
  {  480, "/Game/Carla/Blueprints/Vehicles/JeepWranglerRubicon/BP_JeepWranglerRubicon" },
  {  480, "/Game/Carla/Blueprints/Vehicles/BmwGrandTourer/BP_BmwGrandTourer" },
  {  485, "/Game/Carla/Blueprints/Vehicles/Mustang/BP_Mustang" },
  {  485, "/Game/Carla/Blueprints/Vehicles/Tesla/BP_Tesla" },
  {  490, "/Game/Carla/Blueprints/Vehicles/AudiETron/BP_Audi_Etron" },
  {  492, "/Game/Carla/Blueprints/Vehicles/LincolnMKZ/BP_LincolnMKZ" },
  {  510, "/Game/Carla/Blueprints/Vehicles/DodgeCharger/BP_DodgeCharger" },
  {  510, "/Game/Carla/Blueprints/Vehicles/MercedesCCC/BP_MercedesCCC" },
  {  520, "/Game/Carla/Blueprints/Vehicles/ChevroletImpala/BP_ChevroletImpala" },
  {  525, "/Game/Carla/Blueprints/Vehicles/FordCrown01/BP_FordCrown01" },
  {  525, "/Game/Carla/Blueprints/Vehicles/NissanPatrol/BP_NissanPatrol" },
  {  580, "/Game/Carla/Blueprints/Vehicles/Cybertruck/BP_Cybertruck" },
  {  600, "/Game/Carla/Blueprints/Vehicles/MercedesSprinter/BP_MercedesSprinter" },
  {  600, "/Game/Carla/Blueprints/Vehicles/Ambulance/BP_Ambulance" },
  {  700, "/Game/Carla/Blueprints/Vehicles/MitsubishiFusoRosa/BP_MitsubishiFusoRosa" },
  {  700, "/Game/Carla/Blueprints/Vehicles/EuropeanHGV/BP_EuropeanHGV" },
  {  750, "/Game/Carla/Blueprints/Vehicles/CarlaCola/BP_CarlaCola" },
  { 1000, "/Game/Carla/Blueprints/Vehicles/ActrosFiretruck/BP_ActrosFiretruck" },
  { 1300, "/Game/Carla/Blueprints/Vehicles/MiningTruck/BP_MiningTruck" },
};
}  // namespace

QString pickClosestBaseVehicleBP(float lengthCm) {
  float bestDiff = 1e9f;
  QString bestPath = "/Game/Carla/Blueprints/USDImportTemplates/BaseUSDImportVehicle";
  for (const auto &p : kProtos) {
    const float d = std::abs(p.lengthCm - lengthCm);
    if (d < bestDiff) { bestDiff = d; bestPath = QString::fromLatin1(p.path); }
  }
  return bestPath;
}

}  // namespace carla_studio::vehicle_import
