#pragma once
#include <stdint.h>
#include <vector>
#include <cstring>

class ITSContainer
{
    
public:

    typedef bool OptionalValueAvailable_t;
    
    /* Latitude Dependencies*/
    typedef enum Latitude
    {
        Latitude_oneMicroDegreeNorth = 10,
        Latitude_oneMicroDegreeSouth = -10,
        Latitude_unavailable = 900000001
    } e_Latitude;

    /* Latitude */
    typedef long Latitude_t;

    /* Longitude Dependencies */
    typedef enum Longitude
    {
        Longitude_oneMicroDegreeEast = 10,
        Longitude_oneMicroDegreeWest = -10,
        Longitude_unavailable = 1800000001
    } e_Longitude;

    /* Longitude */
    typedef long Longitude_t;

    /* SemiAxisLength Dependencies */
    typedef enum SemiAxisLength
    {
        SemiAxisLength_oneCentimeter = 1,
        SemiAxisLength_outOfRange = 4094,
        SemiAxisLength_unavailable = 4095
    } e_SemiAxisLength;

    /* SemiAxisLength*/
    typedef long SemiAxisLength_t;

    /* HeadingValue Dependencies */
    typedef enum HeadingValue
    {
        HeadingValue_wgs84North = 0,
        HeadingValue_wgs84East = 900,
        HeadingValue_wgs84South = 1800,
        HeadingValue_wgs84West = 2700,
        HeadingValue_unavailable = 3601
    } e_HeadingValue;

    /* HeadingValue */
    typedef long HeadingValue_t;

     /* HeadingConfidence Dependencies */
    typedef enum HeadingConfidence {
        HeadingConfidence_equalOrWithinZeroPointOneDegree   = 1,
        HeadingConfidence_equalOrWithinOneDegree    = 10,
        HeadingConfidence_outOfRange    = 126,
        HeadingConfidence_unavailable   = 127
    } e_HeadingConfidence;
 
    /* HeadingConfidence */
    typedef long HeadingConfidence_t;

    /* PosConfidenceEllipse*/
    typedef struct PosConfidenceEllipse
    {
        SemiAxisLength_t semiMajorConfidence;
        SemiAxisLength_t semiMinorConfidence;
        HeadingValue_t semiMajorOrientation;
    } PosConfidenceEllipse_t;
    
    /* AltitudeValue Dependencies */
    typedef enum AltitudeValue
    {
        AltitudeValue_referenceEllipsoidSurface = 0,
        AltitudeValue_oneCentimeter = 1,
        AltitudeValue_unavailable = 800001
    } e_AltitudeValue;

    /* AltitudeValue */
    typedef long AltitudeValue_t;

    /* AltitudeConfidence Dependencies */
    typedef enum AltitudeConfidence
    {
        AltitudeConfidence_alt_000_01   = 0,
        AltitudeConfidence_alt_000_02   = 1,
        AltitudeConfidence_alt_000_05   = 2,
        AltitudeConfidence_alt_000_10   = 3,
        AltitudeConfidence_alt_000_20   = 4,
        AltitudeConfidence_alt_000_50   = 5,
        AltitudeConfidence_alt_001_00   = 6,
        AltitudeConfidence_alt_002_00   = 7,
        AltitudeConfidence_alt_005_00   = 8,
        AltitudeConfidence_alt_010_00   = 9,
        AltitudeConfidence_alt_020_00   = 10,
        AltitudeConfidence_alt_050_00   = 11,
        AltitudeConfidence_alt_100_00   = 12,
        AltitudeConfidence_alt_200_00   = 13,
        AltitudeConfidence_outOfRange   = 14,
        AltitudeConfidence_unavailable  = 15
    }e_AltitudeConfidence;

    /* AltitudeConfidence */
    typedef long AltitudeConfidence_t;

    /* Altitude */
    typedef struct Altitude
    {
        AltitudeValue_t altitudeValue;
        AltitudeConfidence_t altitudeConfidence;
    }Altitude_t;

    /* ReferencePosition */
    typedef struct ReferencePosition
    {
        Latitude_t latitude;
        Longitude_t longitude;
        PosConfidenceEllipse_t positionConfidenceEllipse;
        Altitude_t altitude;
    } ReferencePosition_t;

    /* StationType Dependencies */
    typedef enum StationType
    {
        StationType_unknown = 0,
        StationType_pedestrian = 1,
        StationType_cyclist = 2,
        StationType_moped = 3,
        StationType_motorcycle = 4,
        StationType_passengerCar = 5,
        StationType_bus = 6,
        StationType_lightTruck = 7,
        StationType_heavyTruck = 8,
        StationType_trailer = 9,
        StationType_specialVehicles = 10,
        StationType_tram = 11,
        StationType_roadSideUnit = 15        
    } e_StationType;

    /* StationType */
    typedef long StationType_t;

    /* StationID*/
    typedef long StationID_t;
    // typedef unsigned long StationID_t;

    /* Dependencies */
    typedef enum protocolVersion
    {
        protocolVersion_currentVersion = 1
    } e_protocolVersion;

    typedef enum messageID
    {
        messageID_custom = 0,
        messageID_denm = 1,
        messageID_cam = 2,
        messageID_poi = 3,
        messageID_spat = 4,
        messageID_map = 5,
        messageID_ivi = 6,
        messageID_ev_rsr = 7
    } e_messageID;

    typedef struct ItsPduHeader
    {
        long protocolVersion;
        long messageID;
        StationID_t stationID;
    } ItsPduHeader_t;

    /* Heading */
    typedef struct Heading
    {
        HeadingValue_t headingValue;
        HeadingConfidence_t headingConfidence;
    } Heading_t;

    /* SpeedValue Dependencies */
    typedef enum SpeedValue
    {
        SpeedValue_standstill = 0,
        SpeedValue_oneCentimeterPerSec = 1,
        SpeedValue_unavailable = 16383
    } e_SpeedValue;

    /* SpeedValue */
    typedef long SpeedValue_t;

    /* SpeedConfidence Dependencies */
    typedef enum SpeedConfidence
    {
        SpeedConfidence_equalOrWithInOneCentimerterPerSec = 1,
        SpeedConfidence_equalOrWithinOneMeterPerSec = 100,
        SpeedConfidence_outOfRange = 126,
        SpeedConfidence_unavailable = 127
    } e_SpeedConfidence;

    /* SpeedConfidence */
    typedef long SpeedConfidence_t;

    /* Speed */
    typedef struct speed
    {
        SpeedValue_t speedValue;
        SpeedConfidence_t speedConfidence;
    } Speed_t;

    /* DriveDirection Dependencies */
    typedef enum DriveDirection 
    {
        DriveDirection_forward  = 0,
        DriveDirection_backward = 1,
        DriveDirection_unavailable  = 2
    } e_DriveDirection;
 
    /* DriveDirection */
    typedef long DriveDirection_t;

    /* VehicleLength Dependencies */
    typedef enum VehicleLengthValue
    {
        VehicleLengthValue_tenCentimeters = 1,
        VehicleLengthValue_outOfRange = 1022,
        VehicleLengthValue_unavailable = 1023
    } e_VehicleLengthValue;

    /* VehicleLengthValue */
    typedef long VehicleLengthValue_t;

    /* VehicleLengthConfidenceIndication Dependencies */
    typedef enum VehicleLengthConfidenceIndication
    {
        VehicleLengthConfidenceIndication_noTrailerPresent = 0,
        VehicleLengthConfidenceIndication_trailerPresentWithKnownLength = 1,
        VehicleLengthConfidenceIndication_trailerPresentWithUnknownLength  = 2,
        VehicleLengthConfidenceIndication_trailerPresenceIsUnknown = 3,
        VehicleLengthConfidenceIndication_unavailable = 4
    } e_VehicleLengthConfidenceIndication;

    /* VehicleLengthConfidenceIndication */
    typedef long VehicleLengthConfidenceIndication_t;

    /* VehicleLength */
    typedef struct VehicleLength
    {
        VehicleLengthValue_t vehicleLengthValue;
        VehicleLengthConfidenceIndication_t vehicleLengthConfidenceIndication;
    } VehicleLength_t;

    /* VehicleWidth Dependencies */
    typedef enum VehicleWidth
    {
        VehicleWidth_tenCentimeters = 1,
        VehicleWidth_outOfRange = 61,
        VehicleWidth_unavailable = 62
    } e_VehicleWidth;

    /* VehicleWidth */
    typedef long VehicleWidth_t;

    /* LongitudinalAcceleration Dependencies */
    typedef enum LongitudinalAcceletationValue
    {
        LongitudinalAccelerationValue_pointOneMeterPerSecSquaredForward = 1,
        LongitudinalAccelerationValue_pointOneMeterPerSecSquaredBackWard = -1,
        LongitudinalAccelerationValue_unavailable = 161
    } e_LongitudinalAccelerationValue;

    /* LongitudinalAcclerationValue */
    typedef long LongitudinalAccelerationValue_t;

    /* AccelerationConfidence Dependencies */
    typedef enum AccelerationConfidence
    {
        AccelerationConfindence_pointOneMeterPerSecSquared = 1,
        AccelerationConfidence_outOfRange = 101,
        AccelerationConfidence_unavailable = 102
    } e_AccelerationConfidence;

    /* AccelerationConfidence*/
    typedef long AccelerationConfidence_t;

    /* LongitudinalAcceleration */
    typedef struct LongitudinalAcceleration
    {
        LongitudinalAccelerationValue_t longitudinalAccelerationValue;
        AccelerationConfidence_t longitudinalAccelerationConfidence;
    } LongitudinalAcceleration_t;

    /* CurvatureValue Dependencies */
    typedef enum CurvatureValue
    {
        CurvatureValue_straight = 0,
        CurvatureValue_reciprocalOf1MeterRadiusToRight = -30000,
        CurvatureValue_reciprocalOf1MeterRadiusToLeft = 30000,
        CurvatureValue_unavailable = 30001
    } e_CurvatureValue;

    /* CurvatureValue */
    typedef long CurvatureValue_t;

    /* CurvatureConfidence Dependencies*/
    typedef enum CurvatureConfidence
    {
        CurvatureConfidence_onePerMeter_0_00002 = 0,
        CurvatureConfidence_onePerMeter_0_0001  = 1,
        CurvatureConfidence_onePerMeter_0_0005  = 2,
        CurvatureConfidence_onePerMeter_0_002   = 3,
        CurvatureConfidence_onePerMeter_0_01    = 4,
        CurvatureConfidence_onePerMeter_0_1 = 5,
        CurvatureConfidence_outOfRange  = 6,
        CurvatureConfidence_unavailable = 7
    } e_CurvatureConfidence;

    /* CurvatureConfidence */
    typedef long CurvatureConfidence_t;

    /* Curvature */
    typedef struct Curvature
    {
        CurvatureValue_t curvatureValue;
        CurvatureConfidence_t curvatureConfidence;
    } Curvature_t;

    /* CurvatureCalculationMode Dependencies */
    typedef enum CurvatureCalculationMode
    {
        CurvatureCalculationMode_yarRateUsed = 0,
        CurvatureCalculationMode_yarRateNotUsed = 1,
        CurvatureCalculationMode_unavailable = 2
    } e_CurvatureCalculationMode;

    /* CurvatureCalculationMode */
    typedef long CurvatureCalculationMode_t;

    /* YawRateValue Dependencies */
    typedef enum YawRateValue
    {
        YawRateValue_straight = 0,
        YawRateValue_degSec_000_01ToRight   = -1,
        YawRateValue_degSec_000_01ToLeft    = 1,
        YawRateValue_unavailable    = 32767
    } e_YawRateValue;

    /* YawRateValue */
    typedef long YawRateValue_t;

    /* YawRateConfidence Dependencies */
    typedef enum YawRateConfidence {
        YawRateConfidence_degSec_000_01 = 0,
        YawRateConfidence_degSec_000_05 = 1,
        YawRateConfidence_degSec_000_10 = 2,
        YawRateConfidence_degSec_001_00 = 3,
        YawRateConfidence_degSec_005_00 = 4,
        YawRateConfidence_degSec_010_00 = 5,
        YawRateConfidence_degSec_100_00 = 6,
        YawRateConfidence_outOfRange    = 7,
        YawRateConfidence_unavailable   = 8
    } e_YawRateConfidence;
 
    /* YawRateConfidence */
    typedef long YawRateConfidence_t;

    /* YawRate */
    typedef struct YawRate
    {
        YawRateValue_t yawRateValue;
        YawRateConfidence_t yawRateConfidence;
    } YawRate_t;

    /* AccelerationControl Dependencies */
    typedef enum AccelerationControl {
        AccelerationControl_brakePedalEngaged   = 0,
        AccelerationControl_gasPedalEngaged = 1,
        AccelerationControl_emergencyBrakeEngaged   = 2,
        AccelerationControl_collisionWarningEngaged = 3,
        AccelerationControl_accEngaged  = 4,
        AccelerationControl_cruiseControlEngaged    = 5,
        AccelerationControl_speedLimiterEngaged = 6
    } e_AccelerationControl;
 
    /* AccelerationControl */
    typedef uint8_t AccelerationControl_t;

    /* LanePosition Dependencies */
    typedef enum LanePosition {
        LanePosition_offTheRoad = -1,
        LanePosition_hardShoulder   = 0,
        LanePosition_outermostDrivingLane   = 1,
        LanePosition_secondLaneFromOutside  = 2
    } e_LanePosition;
 
    /* LanePosition */
    typedef long LanePosition_t;

    /* SteeringWheelAngleValue Dependencies */
    typedef enum SteeringWheelAngleValue {
        SteeringWheelAngleValue_straight    = 0,
        SteeringWheelAngleValue_onePointFiveDegreesToRight  = -1,
        SteeringWheelAngleValue_onePointFiveDegreesToLeft   = 1,
        SteeringWheelAngleValue_unavailable = 512
    } e_SteeringWheelAngleValue;
 
    /* SteeringWheelAngleValue */
    typedef long SteeringWheelAngleValue_t;

    /* SteeringWheelAngleConfidence Dependencies */
    typedef enum SteeringWheelAngleConfidence {
        SteeringWheelAngleConfidence_equalOrWithinOnePointFiveDegree    = 1,
        SteeringWheelAngleConfidence_outOfRange = 126,
        SteeringWheelAngleConfidence_unavailable    = 127
    } e_SteeringWheelAngleConfidence;
 
    /* SteeringWheelAngleConfidence */
    typedef long SteeringWheelAngleConfidence_t;

    /* SteeringWheelAngle */
    typedef struct SteeringWheelAngle
    {
        SteeringWheelAngleValue_t steeringWheelAngleValue;
        SteeringWheelAngleConfidence_t steeringWheelAngleConfidence;
    } SteeringWheelAngle_t;

    /* LateralAccelerationValue Dependencies */
    typedef enum LateralAccelerationValue {
        LateralAccelerationValue_pointOneMeterPerSecSquaredToRight  = -1,
        LateralAccelerationValue_pointOneMeterPerSecSquaredToLeft   = 1,
        LateralAccelerationValue_unavailable    = 161
    } e_LateralAccelerationValue;

    /* LateralAccelerationValue */
    typedef long LateralAccelerationValue_t;

    /* LateralAcceleration */
    typedef struct LateralAcceleration
    {
        LateralAccelerationValue_t lateralAccelerationValue;
        AccelerationConfidence_t lateralAccelerationConfidence;
    } LateralAcceleration_t;

    /* VerticalAccelerationValue Dependencies */
    typedef enum VerticalAccelerationValue {
        VerticalAccelerationValue_pointOneMeterPerSecSquaredUp  = 1,
        VerticalAccelerationValue_pointOneMeterPerSecSquaredDown    = -1,
        VerticalAccelerationValue_unavailable   = 161
    } e_VerticalAccelerationValue;
 
    /* VerticalAccelerationValue */
    typedef long VerticalAccelerationValue_t;

    /* VerticalAcceleration */
    typedef struct VerticalAcceleration
    {
        VerticalAccelerationValue_t verticalAccelerationValue;
        AccelerationConfidence_t verticalAccelerationConfidence;
    } VerticalAcceleration_t;

    /* PerformanceClass Dependencies */
    typedef enum PerformanceClass {
        PerformanceClass_unavailable    = 0,
        PerformanceClass_performanceClassA  = 1,
        PerformanceClass_performanceClassB  = 2
    } e_PerformanceClass;
 
    /* PerformanceClass */
    typedef long PerformanceClass_t;

    /* ProtectedZoneID */
    typedef long ProtectedZoneID_t;

    /* CenDsrcTollingZoneID */
    typedef ProtectedZoneID_t CenDsrcTollingZoneID_t;

    /* CenDsrcTollingZone */
    typedef struct CenDsrcTollingZone {
        Latitude_t   protectedZoneLatitude;
        Longitude_t  protectedZoneLongitude;
        CenDsrcTollingZoneID_t  cenDsrcTollingZoneID;   /* OPTIONAL */
        OptionalValueAvailable_t cenDsrcTollingZoneIDAvailable;
    } CenDsrcTollingZone_t;

    /* ProtectedZoneType Dependencies */
    typedef enum ProtectedZoneType {
        ProtectedZoneType_cenDsrcTolling    = 0

    } e_ProtectedZoneType;
 
    /* ProtectedZoneType */
    typedef long     ProtectedZoneType_t;

    /* TimestampIts Dependencies */
    typedef enum TimestampIts {
        TimestampIts_utcStartOf2004 = 0,
        TimestampIts_oneMillisecAfterUTCStartOf2004 = 1
    } e_TimestampIts;



    /* TimestampIts */
    typedef long TimestampIts_t; 

    /* ProtectedZoneRadius Dependencies */
    typedef enum ProtectedZoneRadius {
        ProtectedZoneRadius_oneMeter    = 1
    } e_ProtectedZoneRadius;
 
    /* ProtectedZoneRadius */
    typedef long ProtectedZoneRadius_t;

    /* ProtectedCommunicationZone */
    typedef struct ProtectedCommunicationZone {
        ProtectedZoneType_t  protectedZoneType;
        TimestampIts_t  expiryTime /* OPTIONAL */;
        OptionalValueAvailable_t expiryTimeAvailable;
        Latitude_t   protectedZoneLatitude;
        Longitude_t  protectedZoneLongitude;
        ProtectedZoneRadius_t   protectedZoneRadius    /* OPTIONAL */;
        OptionalValueAvailable_t protectedZoneRadiusAvailable;
        ProtectedZoneID_t   protectedZoneID    /* OPTIONAL */;
        OptionalValueAvailable_t protectedZoneIDAvailable;
    } ProtectedCommunicationZone_t;

    /* ProtectedCommunicationZonesRSU */
    typedef struct ProtectedCommunicationZonesRSU {
        long ProtectedCommunicationZoneCount;
        std::vector<ProtectedCommunicationZone_t> list;
    } ProtectedCommunicationZonesRSU_t;

    /* VehicleRole Dependencies */
    typedef enum VehicleRole {
        VehicleRole_default = 0,
        VehicleRole_publicTransport = 1,
        VehicleRole_specialTransport    = 2,
        VehicleRole_dangerousGoods  = 3,
        VehicleRole_roadWork    = 4,
        VehicleRole_rescue  = 5,
        VehicleRole_emergency   = 6,
        VehicleRole_safetyCar   = 7,
        VehicleRole_agriculture = 8,
        VehicleRole_commercial  = 9,
        VehicleRole_military    = 10,
        VehicleRole_roadOperator    = 11,
        VehicleRole_taxi    = 12,
        VehicleRole_reserved1   = 13,
        VehicleRole_reserved2   = 14,
        VehicleRole_reserved3   = 15
    } e_VehicleRole;
 
    /* VehicleRole */
    typedef long VehicleRole_t;

    /* ExteriorLights Dependencies */
    typedef enum ExteriorLights {
        ExteriorLights_lowBeamHeadlightsOn  = 0,
        ExteriorLights_highBeamHeadlightsOn = 1,
        ExteriorLights_leftTurnSignalOn = 2,
        ExteriorLights_rightTurnSignalOn    = 3,
        ExteriorLights_daytimeRunningLightsOn   = 4,
        ExteriorLights_reverseLightOn   = 5,
        ExteriorLights_fogLightOn   = 6,
        ExteriorLights_parkingLightsOn  = 7
    } e_ExteriorLights;

    /* ExteriorLights */
    typedef uint8_t ExteriorLights_t;
    /* DeltaLatitude Dependencies */
    typedef enum DeltaLatitude {
        DeltaLatitude_oneMicrodegreeNorth   = 10,
        DeltaLatitude_oneMicrodegreeSouth   = -10,
        DeltaLatitude_unavailable   = 131072
    } e_DeltaLatitude;
 
    /* DeltaLatitude */
    typedef long DeltaLatitude_t;

    /* DeltaLongitude Dependencies */
    typedef enum DeltaLongitude {
        DeltaLongitude_oneMicrodegreeEast   = 10,
        DeltaLongitude_oneMicrodegreeWest   = -10,
        DeltaLongitude_unavailable  = 131072
    } e_DeltaLongitude;
 
    /* DeltaLongitude */
    typedef long DeltaLongitude_t;

    /* DeltaAltitude Dependencies */
    typedef enum DeltaAltitude {
        DeltaAltitude_oneCentimeterUp   = 1,
        DeltaAltitude_oneCentimeterDown = -1,
        DeltaAltitude_unavailable   = 12800
    } e_DeltaAltitude;
 
    /* DeltaAltitude */
    typedef long DeltaAltitude_t;

    /* DeltaReferencePosition */
    typedef struct DeltaReferencePosition {
        DeltaLatitude_t  deltaLatitude;
        DeltaLongitude_t     deltaLongitude;
        DeltaAltitude_t  deltaAltitude;
    } DeltaReferencePosition_t;

    /* PathDeltaTime Dependencies */
    typedef enum PathDeltaTime {
        PathDeltaTime_tenMilliSecondsInPast = 1
    } e_PathDeltaTime;
 
    /* PathDeltaTime */
    typedef long PathDeltaTime_t;

    /* PathPoint */
    typedef struct PathPoint {
        DeltaReferencePosition_t     pathPosition;
        PathDeltaTime_t *pathDeltaTime  /* OPTIONAL */;

    } PathPoint_t;

    /* PathHistory */
    typedef struct PathHistory {
        long NumberOfPathPoint;
        std::vector<PathPoint_t> data;
    } PathHistory_t;
};

class CAMContainer
{
public:

    /* GenerationDeltaTime Dependencies*/
    typedef enum GenerationDeltaTime
    {
        GenerationDeltaTime_oneMilliSec = 1
    } e_GenerationDeltaTime;

    /* GenerationDeltaTime */
    typedef long GenerationDeltaTime_t;

    /* BasicContainer */
    typedef struct BasicContainer
    {
        ITSContainer::StationType_t stationType;
        ITSContainer::ReferencePosition_t referencePosition;
    } BasicContainer_t;

    /* HighFrequencyContainer Dependencies */
    typedef enum HighFrequencyContainer_PR : long
    {
        HighFrequencyContainer_PR_NOTHING, /* No components present */
        HighFrequencyContainer_PR_basicVehicleContainerHighFrequency,
        HighFrequencyContainer_PR_rsuContainerHighFrequency
    } HighFrequencyContainer_PR;

    typedef bool OptionalStructAvailable_t;

    /* BasicVehicleContainerHighFrequency*/
    typedef struct BasicVehicleContainerHighFrequency
    {
        ITSContainer::Heading_t heading;
        ITSContainer::Speed_t speed;
        ITSContainer::DriveDirection_t driveDirection;
        ITSContainer::VehicleLength_t vehicleLength;
        ITSContainer::VehicleWidth_t vehicleWidth;
        ITSContainer::LongitudinalAcceleration_t longitudinalAcceleration;
        ITSContainer::Curvature_t curvature;
        ITSContainer::CurvatureCalculationMode_t   curvatureCalculationMode;
        ITSContainer::YawRate_t    yawRate;
        
        OptionalStructAvailable_t accelerationControlAvailable;
        ITSContainer::AccelerationControl_t   accelerationControl    /* OPTIONAL */;
        
        OptionalStructAvailable_t lanePositionAvailable;
        ITSContainer::LanePosition_t lanePosition   /* OPTIONAL */;
        
        OptionalStructAvailable_t steeringWheelAngleAvailable;
        ITSContainer::SteeringWheelAngle_t   steeringWheelAngle /* OPTIONAL */;

        OptionalStructAvailable_t lateralAccelerationAvailable;
        ITSContainer::LateralAcceleration_t  lateralAcceleration    /* OPTIONAL */;

        OptionalStructAvailable_t verticalAccelerationAvailable;
        ITSContainer::VerticalAcceleration_t verticalAcceleration   /* OPTIONAL */;

        OptionalStructAvailable_t performanceClassAvailable;
        ITSContainer::PerformanceClass_t performanceClass   /* OPTIONAL */;

        OptionalStructAvailable_t cenDsrcTollingZoneAvailable;
        ITSContainer::CenDsrcTollingZone_t   cenDsrcTollingZone /* OPTIONAL */;
        
    } BasicVehicleContainerHighFrequency_t;

    /* RsuContainerHighFrequency */
    typedef struct RSUContainerHighFrequency
    {
        ITSContainer::ProtectedCommunicationZonesRSU_t protectedCommunicationZonesRSU;
    } RSUContainerHighFrequency_t;

    /* HighFrequencyContainer */
    typedef struct HighFrequencyContainer
    {
        HighFrequencyContainer_PR present;

        BasicVehicleContainerHighFrequency_t basicVehicleContainerHighFrequency;
        RSUContainerHighFrequency_t rsuContainerHighFrequency;

    } HighFrequencyContainer_t;

    /* Dependencies */
    typedef enum LowFrequencyContainer_PR : long
    {
        LowFrequencyContainer_PR_NOTHING,   /* No components present */
        LowFrequencyContainer_PR_basicVehicleContainerLowFrequency,
        /* Extensions may appear below */
     
    } LowFrequencyContainer_PR;

    /* BasicVehicleContainerLowFrequency */
    typedef struct BasicVehicleContainerLowFrequency {
        ITSContainer::VehicleRole_t    vehicleRole;
        ITSContainer::ExteriorLights_t     exteriorLights;
        ITSContainer::PathHistory_t    pathHistory;
    } BasicVehicleContainerLowFrequency_t;

    /* LowFrequencyContainer */
    typedef struct LowFrequencyContainer
    {
        LowFrequencyContainer_PR present;
        // Since only option is available
        BasicVehicleContainerLowFrequency_t basicVehicleContainerLowFrequency;

    } LowFrequencyContainer_t;

    /* CamParameters */
    typedef struct CamParameters
    {
        BasicContainer_t basicContainer;
        HighFrequencyContainer_t highFrequencyContainer;
        LowFrequencyContainer_t lowFrequencyContainer; /* OPTIONAL */
        // Optional TODO: SpecialVehicleContainer *specialVehicleContainer
    } CamParameters_t;

    /* CoopAwareness*/
    typedef struct CoopAwareness 
    {
        GenerationDeltaTime_t generationDeltaTime;
        CamParameters_t camParameters;
    } CoopAwareness_t;


};

    /* CoopAwareness */
    typedef struct CAM
    {
        ITSContainer::ItsPduHeader_t header;
        CAMContainer::CoopAwareness_t cam;
    } CAM_t;


    typedef struct CustomV2XM
    {
        ITSContainer::ItsPduHeader_t header;
        char message[100];
    } CustomV2XM_t;
