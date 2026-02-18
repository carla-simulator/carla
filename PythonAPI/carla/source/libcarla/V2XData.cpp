// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the 
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <iostream>
#include <ostream>
#include <vector>
#include <Python.h>

#include <carla/sensor/data/LibITS.h>
#include <carla/sensor/data/V2XData.h>
#include <carla/sensor/data/V2XEvent.h>

/**********************************************************************************************/
// Functions to convert CAM message from V2X sensor to python dict
std::string GetStationTypeString(const ITSContainer::StationType_t stationType)
{
    switch (stationType)
    {
    case ITSContainer::StationType_unknown:
        return "Uunknown";
    case ITSContainer::StationType_pedestrian:
        return "Pedestrian";
    case ITSContainer::StationType_cyclist:
        return "Cyclist";
    case ITSContainer::StationType_moped:
        return "Moped";
    case ITSContainer::StationType_motorcycle:
        return "Motorcycle";
    case ITSContainer::StationType_passengerCar:
        return "Passenger Car";
    case ITSContainer::StationType_bus:
        return "Bus";
    case ITSContainer::StationType_lightTruck:
        return "Light Truck";
    case ITSContainer::StationType_heavyTruck:
        return "HeavyTruck";
    case ITSContainer::StationType_trailer:
        return "Trailer";
    case ITSContainer::StationType_specialVehicles:
        return "Special Vehicle";
    case ITSContainer::StationType_tram:
        return "Tram";
    case ITSContainer::StationType_roadSideUnit:
        return "Road Side Unit";
    default:
        return "Unknown";
    }
}
std::string GetSemiConfidenceString(const long confidence)
{
  switch(confidence)
  {
    case ITSContainer::SemiAxisLength_oneCentimeter:
      return "Semi Axis Length One Centimeter";
    case ITSContainer::SemiAxisLength_outOfRange:
      return "Semi Axis Length Out Of Range";
    default:
      return "Semi Axis Length Unavailable";
  }
}

std::string GetSemiOrientationString(const long orientation)
{
  switch(orientation)
  {
    case ITSContainer::HeadingValue_wgs84North:
      return "Heading wgs84 North";
    case ITSContainer::HeadingValue_wgs84East:
      return "Heading wgs84 East";
    case ITSContainer::HeadingValue_wgs84South:
      return "Heading wgs84 South";
    case ITSContainer::HeadingValue_wgs84West:
      return "Heading wgs84 West";
    default:
      return "Heading Unavailable";
  }
}

std::string GetAltitudeConfidenceString(const ITSContainer::AltitudeConfidence_t altitudeConfidence)
{
  switch(altitudeConfidence)
  {
    case ITSContainer::AltitudeConfidence_alt_000_01:
      return "AltitudeConfidence_alt_000_01";
    case ITSContainer::AltitudeConfidence_alt_000_02:
      return "AltitudeConfidence_alt_000_02";
    case ITSContainer::AltitudeConfidence_alt_000_05:
      return "AltitudeConfidence_alt_000_05";
    case ITSContainer::AltitudeConfidence_alt_000_10:
      return "AltitudeConfidence_alt_000_10";
    case ITSContainer::AltitudeConfidence_alt_000_20:
      return "AltitudeConfidence_alt_000_20";
    case ITSContainer::AltitudeConfidence_alt_000_50:
      return "AltitudeConfidence_alt_000_50";
    case ITSContainer::AltitudeConfidence_alt_001_00:
      return "AltitudeConfidence_alt_001_00";
    case ITSContainer::AltitudeConfidence_alt_002_00:
      return "AltitudeConfidence_alt_002_00";
    case ITSContainer::AltitudeConfidence_alt_005_00:
      return "AltitudeConfidence_alt_005_00";
    case ITSContainer::AltitudeConfidence_alt_010_00:
      return "AltitudeConfidence_alt_010_00";
    case ITSContainer::AltitudeConfidence_alt_020_00:
      return "AltitudeConfidence_alt_020_00";
    case ITSContainer::AltitudeConfidence_alt_050_00:
      return "AltitudeConfidence_alt_050_00";
    case ITSContainer::AltitudeConfidence_alt_100_00:
      return "AltitudeConfidence_alt_100_00";
    case ITSContainer::AltitudeConfidence_alt_200_00:
      return "AltitudeConfidence_alt_200_00";
    case ITSContainer::AltitudeConfidence_outOfRange:
      return "AltitudeConfidence_alt_outOfRange";
    default:
      return "AltitudeConfidence_unavailable";
  }
}
static boost::python::dict GetReferenceContainer(const CAM_t &message)
{
  boost::python::dict ReferencePosition;
  ITSContainer::ReferencePosition_t ref = message.cam.camParameters.basicContainer.referencePosition;
  ReferencePosition["Latitude"] = ref.latitude;
  ReferencePosition["Longitude"] = ref.longitude;
  boost::python::dict PosConfidence;
  PosConfidence["Semi Major Confidence"] = GetSemiConfidenceString(ref.positionConfidenceEllipse.semiMajorConfidence);
  PosConfidence["Semi Minor Confidence"] = GetSemiConfidenceString(ref.positionConfidenceEllipse.semiMinorConfidence);
  PosConfidence["Semi Major Orientation"] = GetSemiOrientationString(ref.positionConfidenceEllipse.semiMajorOrientation);
  ReferencePosition["Position Confidence Eliipse"] = PosConfidence;
  boost::python::dict Altitude;
  Altitude["Altitude Value"] = ref.altitude.altitudeValue;
  Altitude["Altitude Confidence"] = GetAltitudeConfidenceString(ref.altitude.altitudeConfidence);
  return ReferencePosition;
}
static boost::python::dict GetBasicContainer(const CAM_t &message)
{
    boost::python::dict BasicContainer;
    BasicContainer["Station Type"] = GetStationTypeString(message.cam.camParameters.basicContainer.stationType);
    BasicContainer["Reference Position"] = GetReferenceContainer(message);
    return BasicContainer;
}

std::string GetHeadingConfidenceString(const ITSContainer::HeadingConfidence_t confidence)
{
    switch (confidence)
    {
    case ITSContainer::HeadingConfidence_equalOrWithinZeroPointOneDegree:
        return "Equal or With in Zero Point One Degree";
    case ITSContainer::HeadingConfidence_equalOrWithinOneDegree:
        return "Equal or With in One Degree";
    case ITSContainer::HeadingConfidence_outOfRange:
        return "Out of Range";
    default:
        return "Unavailable";
    }
}

std::string GetSpeedConfidenceString(const ITSContainer::SpeedConfidence_t confidence)
{
    switch (confidence)
    {
    case ITSContainer::SpeedConfidence_equalOrWithInOneCentimerterPerSec:
        return "Equal or With in One Centimeter per Sec";
    case ITSContainer::SpeedConfidence_equalOrWithinOneMeterPerSec:
        return "Equal or With in One Meter per Sec";
    case ITSContainer::SpeedConfidence_outOfRange:
        return "Out of Range";
    default:
        return "Unavailable";
    }
}

std::string GetVehicleLengthConfidenceString(const ITSContainer::VehicleLengthConfidenceIndication_t confidence)
{
    switch (confidence)
    {
    case ITSContainer::VehicleLengthConfidenceIndication_noTrailerPresent:
        return "No Trailer Present";
    case ITSContainer::VehicleLengthConfidenceIndication_trailerPresentWithKnownLength:
        return "Trailer Present With Known Length";
    case ITSContainer::VehicleLengthConfidenceIndication_trailerPresentWithUnknownLength:
        return "Trailer Present With Unknown Length";
    case ITSContainer::VehicleLengthConfidenceIndication_trailerPresenceIsUnknown:
        return "Trailer Presence Is Unknown";
    default:
        return "Unavailable";
    }
}

std::string GetAccelerationConfidenceString(const ITSContainer::AccelerationConfidence_t confidence)
{
    switch (confidence)
    {
    case ITSContainer::AccelerationConfindence_pointOneMeterPerSecSquared:
        return "Point One Meter Per Sec Squared";
    case ITSContainer::AccelerationConfidence_outOfRange:
        return "Out Of Range";
    default:
        return "Unavailable";
    }
}

std::string GetCurvatureConfidenceString(const ITSContainer::CurvatureConfidence_t confidence)
{
    switch (confidence)
    {
    case ITSContainer::CurvatureConfidence_onePerMeter_0_00002:
        return "One Per Meter 0_00002";
    case ITSContainer::CurvatureConfidence_onePerMeter_0_0001:
        return "One Per Meter 0_0001";
    case ITSContainer::CurvatureConfidence_onePerMeter_0_0005:
        return "One Per Meter 0_0005";
    case ITSContainer::CurvatureConfidence_onePerMeter_0_002:
        return "One Per Meter 0_002";
    case ITSContainer::CurvatureConfidence_onePerMeter_0_01:
        return "One Per Meter 0_01";
    case ITSContainer::CurvatureConfidence_onePerMeter_0_1:
        return "One Per Meter 0_1";
    case ITSContainer::CurvatureConfidence_outOfRange:
        return "Out Of Range";
    default:
        return "Unavailable";
    }
}

std::string GetYawRateConfidenceString(const ITSContainer::YawRateConfidence_t confidence)
{
    switch (confidence)
    {
    case ITSContainer::YawRateConfidence_degSec_000_01:
        return "degSec 000_01";
    case ITSContainer::YawRateConfidence_degSec_000_05:
        return "degSec 000_05";
    case ITSContainer::YawRateConfidence_degSec_000_10:
        return "degSec 000_10";
    case ITSContainer::YawRateConfidence_degSec_001_00:
        return "degSec 001_00";
    case ITSContainer::YawRateConfidence_degSec_005_00:
        return "degSec 005_00";
    case ITSContainer::YawRateConfidence_degSec_010_00:
        return "degSec 010_00";
    case ITSContainer::YawRateConfidence_degSec_100_00:
        return "degSec 100_00";
    case ITSContainer::YawRateConfidence_outOfRange:
        return "Out Of Range";
    default:
        return "Unavailable";
    }
}

std::string GetSteeringWheelConfidenceString(const ITSContainer::SteeringWheelAngleConfidence_t confidence)
{
    switch (confidence)
    {
    case ITSContainer::SteeringWheelAngleConfidence_equalOrWithinOnePointFiveDegree:
        return "Equal or With in 1.5 degree";
    case ITSContainer::SteeringWheelAngleConfidence_outOfRange:
        return "Out of Range";
    default:
        return "Unavailable";
    }
}

static boost::python::dict GetBVCHighFrequency(const CAM_t &message)
{
    boost::python::dict BVCHighFrequency;
    CAMContainer::BasicVehicleContainerHighFrequency_t bvchf = message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency;
    boost::python::dict HeadingValueConfidence;
    HeadingValueConfidence["Value"] = bvchf.heading.headingValue;
    HeadingValueConfidence["Confidence"] = GetHeadingConfidenceString(bvchf.heading.headingConfidence);
    BVCHighFrequency["Heading"] = HeadingValueConfidence;
    boost::python::dict SpeedValueConfidence;
    SpeedValueConfidence["Value"] = bvchf.speed.speedValue;
    SpeedValueConfidence["Confidence"] = GetSpeedConfidenceString(bvchf.speed.speedConfidence);
    BVCHighFrequency["Speed"] = SpeedValueConfidence;

    BVCHighFrequency["Drive Direction"] = bvchf.driveDirection;

    boost::python::dict VehicleLengthValueConfidence;
    VehicleLengthValueConfidence["Value"] = bvchf.vehicleLength.vehicleLengthValue;
    VehicleLengthValueConfidence["Confidence"] = GetVehicleLengthConfidenceString(bvchf.vehicleLength.vehicleLengthConfidenceIndication);
    BVCHighFrequency["Vehicle Length"] = VehicleLengthValueConfidence;

    BVCHighFrequency["Vehicle Width"] = bvchf.vehicleWidth;

    boost::python::dict LongitudinalAccelerationValueConfidence;
    LongitudinalAccelerationValueConfidence["Value"] = bvchf.longitudinalAcceleration.longitudinalAccelerationValue;
    LongitudinalAccelerationValueConfidence["Confidence"] = GetAccelerationConfidenceString(bvchf.longitudinalAcceleration.longitudinalAccelerationConfidence);
    BVCHighFrequency["Longitudinal Acceleration"] = LongitudinalAccelerationValueConfidence;

    boost::python::dict CurvatureValueConfidence;
    CurvatureValueConfidence["Value"] = bvchf.curvature.curvatureValue;
    CurvatureValueConfidence["Confidence"] = GetCurvatureConfidenceString(bvchf.curvature.curvatureConfidence);
    BVCHighFrequency["Curvature"] = CurvatureValueConfidence;
    BVCHighFrequency["Curvature Calculation Mode"] = bvchf.curvatureCalculationMode;

    boost::python::dict YawValueConfidence;
    YawValueConfidence["Value"] = bvchf.yawRate.yawRateValue;
    YawValueConfidence["Confidence"] = GetYawRateConfidenceString(bvchf.yawRate.yawRateConfidence);
    BVCHighFrequency["Yaw Rate"] = YawValueConfidence;

    boost::python::dict ValueConfidence;
    if (bvchf.accelerationControlAvailable)
    {
        BVCHighFrequency["Acceleration Control"] = bvchf.accelerationControl;
    }
    else
    {
        BVCHighFrequency["Acceleration Control"] = boost::python::object();
    }

    if (bvchf.lanePositionAvailable)
    {
        BVCHighFrequency["Lane Position"] = bvchf.lanePosition;
    }
    else
    {
        BVCHighFrequency["Lane Position"] = boost::python::object();
    }

    if (bvchf.steeringWheelAngleAvailable)
    {
        boost::python::dict ValueConfidence;
        ValueConfidence["Value"] = bvchf.steeringWheelAngle.steeringWheelAngleValue;
        ValueConfidence["Confidence"] = GetSteeringWheelConfidenceString(bvchf.steeringWheelAngle.steeringWheelAngleConfidence);
        BVCHighFrequency["Steering Wheel Angle"] = ValueConfidence;
    }
    else
    {
        BVCHighFrequency["Steering Wheel Angle"] = boost::python::object();
    }

    if (bvchf.lateralAccelerationAvailable)
    {
        boost::python::dict ValueConfidence;
        ValueConfidence["Value"] = bvchf.lateralAcceleration.lateralAccelerationValue;
        ValueConfidence["Confidence"] = GetAccelerationConfidenceString(bvchf.lateralAcceleration.lateralAccelerationConfidence);
        BVCHighFrequency["Lateral Acceleration"] = ValueConfidence;
    }
    else
    {
        BVCHighFrequency["Lateral Acceleration"] = boost::python::object();
    }

    if (bvchf.verticalAccelerationAvailable)
    {
        boost::python::dict ValueConfidence;
        ValueConfidence["Value"] = bvchf.verticalAcceleration.verticalAccelerationValue;
        ValueConfidence["Confidence"] = GetAccelerationConfidenceString(bvchf.verticalAcceleration.verticalAccelerationConfidence);
        BVCHighFrequency["Vertical Acceleration"] = ValueConfidence;
    }
    else
    {
        BVCHighFrequency["Vertical Acceleration"] = boost::python::object();
    }

    if (bvchf.performanceClassAvailable)
    {
        BVCHighFrequency["Performance class"] = bvchf.performanceClass;
    }
    else
    {
        BVCHighFrequency["Performance class"] = boost::python::object();
    }

    if (bvchf.cenDsrcTollingZoneAvailable)
    {
        boost::python::dict ValueConfidence;
        ValueConfidence["Protected Zone Latitude"] = bvchf.cenDsrcTollingZone.protectedZoneLatitude;
        ValueConfidence["Protected Zone Longitude"] = bvchf.cenDsrcTollingZone.protectedZoneLongitude;
        if (bvchf.cenDsrcTollingZone.cenDsrcTollingZoneIDAvailable)
        {
            ValueConfidence["Cen DSRC Tolling Zone ID"] = bvchf.cenDsrcTollingZone.cenDsrcTollingZoneID;
        }
        BVCHighFrequency["Cen DSRC Tolling Zone"] = ValueConfidence;
    }
    else
    {
        BVCHighFrequency["Cen DSRC Tolling Zone"] = boost::python::object();
    }

    return BVCHighFrequency;
}

static boost::python::list GetProtectedCommunicationZone(const CAMContainer::RSUContainerHighFrequency_t &rsuMessage)
{
    boost::python::list PCZlist;

    for (auto i=0u; i< rsuMessage.protectedCommunicationZonesRSU.ProtectedCommunicationZoneCount; ++i) {
        ITSContainer::ProtectedCommunicationZone_t const &data=rsuMessage.protectedCommunicationZonesRSU.data[i];
        boost::python::dict PCZDict;
        PCZDict["Protected Zone Type"] = data.protectedZoneType;
        if (data.expiryTimeAvailable)
        {
            PCZDict["Expiry Time"] = data.expiryTime;
        }

        PCZDict["Protected Zone Latitude"] = data.protectedZoneLatitude;
        PCZDict["Protected Zone Longitude"] = data.protectedZoneLongitude;

        if (data.protectedZoneIDAvailable)
        {
            PCZDict["Protected Zone ID"] = data.protectedZoneID;
        }
        if (data.protectedZoneRadiusAvailable)
        {
            PCZDict["Protected Zone Radius"] = data.protectedZoneRadius;
        }

        PCZlist.append(PCZDict);
    }
    return PCZlist;
}
static boost::python::dict GetRSUHighFrequency(const CAM_t &message)
{
    boost::python::dict RSU;
    CAMContainer::RSUContainerHighFrequency_t rsu = message.cam.camParameters.highFrequencyContainer.rsuContainerHighFrequency;
    RSU["Protected Communication Zone"] = GetProtectedCommunicationZone(rsu);
    return RSU;
}

static boost::python::dict GetHighFrequencyContainer(const CAM_t &message)
{
    boost::python::dict HFC;
    CAMContainer::HighFrequencyContainer_t hfc = message.cam.camParameters.highFrequencyContainer;
    switch (hfc.present)
    {
    case CAMContainer::HighFrequencyContainer_PR_basicVehicleContainerHighFrequency:
        HFC["High Frequency Container Present"] = "Basic Vehicle Container High Frequency";
        HFC["Basic Vehicle Container High Frequency"] = GetBVCHighFrequency(message);
        break;
    case CAMContainer::HighFrequencyContainer_PR_rsuContainerHighFrequency:
        HFC["High Frequency Container Present"] = "RSU Container High Frequency";
        HFC["RSU Container High Frequency"] = GetRSUHighFrequency(message);
        break;
    default:
        HFC["High Frequency Container Present"] = "No container present";
    }
    return HFC;
}

std::string GetVehicleRoleString(const ITSContainer::VehicleRole_t &vehicleRole)
{
    switch (vehicleRole)
    {
    case ITSContainer::VehicleRole_publicTransport:
        return "Public Transport";
    case ITSContainer::VehicleRole_specialTransport:
        return "Special Transport";
    case ITSContainer::VehicleRole_dangerousGoods:
        return "Dangerous Goods";
    case ITSContainer::VehicleRole_roadWork:
        return "Road Work";
    case ITSContainer::VehicleRole_rescue:
        return "Rescue";
    case ITSContainer::VehicleRole_emergency:
        return "Emergency";
    case ITSContainer::VehicleRole_safetyCar:
        return "Safety Car";
    case ITSContainer::VehicleRole_agriculture:
        return "Agriculture";
    case ITSContainer::VehicleRole_commercial:
        return "Commercial";
    case ITSContainer::VehicleRole_military:
        return "Military";
    case ITSContainer::VehicleRole_roadOperator:
        return "Road Operator";
    case ITSContainer::VehicleRole_taxi:
        return "Taxi";
    case ITSContainer::VehicleRole_reserved1:
        return "Reserved 1";
    case ITSContainer::VehicleRole_reserved2:
        return "Reserved 2";
    case ITSContainer::VehicleRole_reserved3:
        return "Reserved 3";
    default:
        return "Default";
    }
}

boost::python::list GetPathHistory(const ITSContainer::PathHistory_t &pathHistory)
{

    boost::python::list PathHistoryList;

    for (auto i=0u; i<pathHistory.NumberOfPathPoint; ++i)
    {
        ITSContainer::PathPoint_t const &pathPoint = pathHistory.data[i];
        boost::python::dict PathHistory;
        PathHistory["Delta Latitude"] = pathPoint.pathPosition.deltaLatitude;
        PathHistory["Delta Longitude"] = pathPoint.pathPosition.deltaLongitude;
        PathHistory["Delta Altitude"] = pathPoint.pathPosition.deltaAltitude;
        if (pathPoint.pathDeltaTime != nullptr)
        {
            PathHistory["Delta Time"] = *pathPoint.pathDeltaTime;
        }
        else
        {
            PathHistory["Delta Time"] = boost::python::object();
        }
        PathHistoryList.append(PathHistory);
    }
    return PathHistoryList;
}
boost::python::dict GetBVCLowFrequency(const CAMContainer::BasicVehicleContainerLowFrequency_t &bvc)
{
    boost::python::dict BVC;
    BVC["Vehicle Role"] = GetVehicleRoleString(bvc.vehicleRole);
    BVC["Exterior Light"] = bvc.exteriorLights;
    if (bvc.pathHistory.NumberOfPathPoint != 0)
    {
        BVC["Path History"] = GetPathHistory(bvc.pathHistory);
    }
    else
    {
        BVC["Path History"] = boost::python::object();
    }
    return BVC;
}

boost::python::dict GetLowFrequencyContainer(const CAM_t &message)
{
    boost::python::dict LFC = boost::python::dict();
    CAMContainer::LowFrequencyContainer_t lfc = message.cam.camParameters.lowFrequencyContainer;
    switch (lfc.present)
    {
    case CAMContainer::LowFrequencyContainer_PR_basicVehicleContainerLowFrequency:
        LFC["Low Frequency Container Present"] = "Basic Vehicle Container Low Frequency";
        LFC["Basic Vehicle Low Frequency"] = GetBVCLowFrequency(lfc.basicVehicleContainerLowFrequency);
        break;
    default:
        LFC["Low Frequency Container Present"] = "No Container Present";
        break;
    }
    return LFC;
}
static boost::python::dict GetCAMParameters(const CAM_t &message)
{
    boost::python::dict CAMParams;
    try
    {

        CAMParams["Basic Container"] = GetBasicContainer(message);
        CAMParams["High Frequency Container"] = GetHighFrequencyContainer(message);
        CAMParams["Low Frequency Container"] = GetLowFrequencyContainer(message);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    return CAMParams;
}

static boost::python::dict GetCoopAwarness(const CAM_t &message)
{

    boost::python::dict Coop;
    CAMContainer::CoopAwareness_t coop = message.cam;
    Coop["Generation Delta Time"] = coop.generationDeltaTime;
    Coop["CAM Parameters"] = GetCAMParameters(message);
    return Coop;
}

std::string GetMessageIDString(const long messageId)
{
    switch (messageId)
    {
    case ITSContainer::messageID_custom:
        return "CUSTOM";
    case ITSContainer::messageID_denm:
        return "DENM";
    case ITSContainer::messageID_cam:
        return "CAM";
    case ITSContainer::messageID_poi:
        return "POI";
    case ITSContainer::messageID_spat:
        return "SPAT";
    case ITSContainer::messageID_map:
        return "MAP";
    case ITSContainer::messageID_ivi:
        return "IVI";
    case ITSContainer::messageID_ev_rsr:
        return "EV_RSR";
    default:
        return "Not Found";
    }
}

static boost::python::object GetCustomV2XBytes(carla::rpc::CustomV2XBytes &self) {
#if PY_MAJOR_VERSION >= 3
  auto *ptr = PyBytes_FromStringAndSize(reinterpret_cast<char *>(self.bytes.data()), self.data_size);
#else
  auto *ptr = PyString_FromStringAndSize(reinterpret_cast<char *>(self.bytes.data()), self.data_size);
#endif
  return boost::python::object(boost::python::handle<>(ptr));
}

static void SetCustomV2XBytes(carla::rpc::CustomV2XBytes &self, boost::python::object const &object) {
  if ( PyObject_CheckBuffer(object.ptr()) == 0 ) {
    std::cerr << "SetCustomV2XBytes: object does not support buffer protocol!" << std::endl;
    return;
  }
  Py_buffer buffer;
  if (PyObject_GetBuffer(object.ptr(), &buffer, PyBUF_SIMPLE) == -1) {
    std::cerr << "SetCustomV2XBytes: PyObject_GetBuffer failed!" << std::endl;
    return;
  }
  self.data_size = static_cast<uint8_t>(self.bytes.max_size());
  auto const object_len = static_cast<size_t>(boost::python::len(object));
  if ( object_len < self.data_size ) {
    self.data_size = static_cast<uint8_t>(object_len);
  }
  PyBuffer_ToContiguous(self.bytes.data(), &buffer, self.data_size, 'C');
  std::fill(self.bytes.begin()+self.data_size, self.bytes.begin()+self.bytes.max_size(), 0);
  PyBuffer_Release(&buffer);
}

static std::string GetCustomV2XString(carla::rpc::CustomV2XBytes const &self) {
  return std::string(reinterpret_cast<char const *>(self.bytes.data()), self.data_size);
}

static void SetCustomV2XString(carla::rpc::CustomV2XBytes &self, std::string const &str) {
  self.data_size = static_cast<uint8_t>(self.bytes.max_size());
  auto const object_len = str.length();
  if ( object_len < self.data_size ) {
    self.data_size = static_cast<uint8_t>(object_len);
  }
  memcpy(self.bytes.data(), str.data(), self.data_size);
  std::fill(self.bytes.begin()+self.data_size, self.bytes.begin()+self.bytes.max_size(), 0);
}

static boost::python::dict GetMessageHeader(const ITSContainer::ItsPduHeader_t &header)
{
    boost::python::dict Header;
    Header["Protocol Version"] = header.protocolVersion;
    Header["Message ID"] = GetMessageIDString(header.messageID);
    Header["Station ID"] = header.stationID;
    return Header;
}

boost::python::dict GetCAMMessage(const carla::sensor::data::CAMData &message)
{
    boost::python::dict CAM;
    CAM_t CAMMessage = message.Message;
    CAM["Header"] = GetMessageHeader(CAMMessage.header);
    CAM["Message"] = GetCoopAwarness(CAMMessage);
    return CAM;
}

boost::python::dict GetCustomV2XBytesDict(carla::rpc::CustomV2XBytes &data)
{
    boost::python::dict CustomV2XBytes;
    CustomV2XBytes["DataSize"] = data.data_size;
    CustomV2XBytes["MaxDataSize"] = data.max_data_size();
    CustomV2XBytes["Bytes"] = GetCustomV2XBytes(data);
    return CustomV2XBytes;
}

boost::python::dict GetCustomV2XMessageDict(const carla::sensor::data::CustomV2XData &message)
{
    boost::python::dict CustomV2X;
    CustomV2XM V2XMessage = message.Message;
    CustomV2X["Header"] = GetMessageHeader(V2XMessage.header);
    CustomV2X["Message"] = GetCustomV2XBytesDict(V2XMessage.data);
    return CustomV2X;
}

static boost::python::dict GetCAMData(const carla::sensor::data::CAMData message)
{
    boost::python::dict myDict;
    try
    {
        myDict["Power"] = message.Power;
        myDict["Message"] = GetCAMMessage(message);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return myDict;
}

static boost::python::dict GetCustomV2XDataDict(const carla::sensor::data::CustomV2XData &message)
{
    boost::python::dict myDict;
    try
    {
        myDict["Power"] = message.Power;
        myDict["Message"] = GetCustomV2XMessageDict(message);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return myDict;
}


namespace carla {
namespace rpc {

std::ostream &operator<<(std::ostream &out, const CustomV2XBytes &bytes) {
    out << "CustomV2XData("
        << "max_data_size=" << std::to_string(bytes.max_data_size())
        << ", data_size=" << std::to_string(bytes.data_size)
        << std::hex << ", bytes=["; 
    for ( size_t i = 0; i < bytes.data_size && i < bytes.max_data_size(); ++i) {
        if ( i > 0 ) {
            out << ", ";
        }
        out << static_cast<int>(bytes.bytes[i]);
    }
    out << "])" << std::dec;
    return out;
}

} // namespace rpc
} // namespace carla

namespace carla {
namespace sensor {
namespace data {

  std::ostream &operator<<(std::ostream &out, const CAMEvent &data) {
    out << "CAMEvent(frame=" << std::to_string(data.GetFrame())
        << ", timestamp=" << std::to_string(data.GetTimestamp())
        << ", message_count=" << std::to_string(data.GetMessageCount())
        << ')';
    return out;
  }

    std::ostream &operator<<(std::ostream &out, const CustomV2XEvent &data) {
    out << "CustomV2XEvent(frame=" << std::to_string(data.GetFrame())
        << ", timestamp=" << std::to_string(data.GetTimestamp())
        << ", message_count=" << std::to_string(data.GetMessageCount())
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const CAMData &data) {
    out << "CAMData(power=" << std::to_string(data.Power)
        << ", stationId=" << std::to_string(data.Message.header.stationID)
        << ", messageId=" << std::to_string(data.Message.header.messageID)
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const CustomV2XData &data) {
    out << "CustomV2XData(power=" << std::to_string(data.Power)
        << ", stationId=" << std::to_string(data.Message.header.stationID)
        << ", messageId=" << std::to_string(data.Message.header.messageID)
        << ", data=" << data.Message.data
        << ')';
    return out;
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
