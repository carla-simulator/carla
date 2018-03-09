// =============================================================================
// -- Unreal classes -----------------------------------------------------------
// =============================================================================

/**
 * @hidden
 * @opt nodefillcolor gray
 */
class UObject {}

/** @opt nodefillcolor gray */ class AAIController extends AController {}
/** @opt nodefillcolor gray */ class AActor extends UObject {}
/** @opt nodefillcolor gray */ class ACharacter extends APawn {}
/** @opt nodefillcolor gray */ class AController extends AActor {}
/** @opt nodefillcolor gray */ class AGameModeBase extends AActor {}
/** @opt nodefillcolor gray */ class AHUD extends AActor {}
/** @opt nodefillcolor gray */ class AInfo extends AActor {}
/** @opt nodefillcolor gray */ class APawn extends AActor {}
/** @opt nodefillcolor gray */ class APlayerController extends AController {}
/** @opt nodefillcolor gray */ class APlayerState extends AInfo {}
/** @opt nodefillcolor gray */ class AWheeledVehicle extends APawn {}
/** @opt nodefillcolor gray */ class UActorComponent extends UObject {}
/** @opt nodefillcolor gray */ class UCameraComponent extends USceneComponent {}
/** @opt nodefillcolor gray */ class UGameInstance extends UObject {}
/** @opt nodefillcolor gray */ class USceneComponent extends UActorComponent {}

// =============================================================================
// -- Agent --------------------------------------------------------------------
// =============================================================================

class UAgentComponent extends USceneComponent {}

class UTrafficSignAgentComponent extends UAgentComponent {}

class UVehicleAgentComponent extends UAgentComponent {}

class UWalkerAgentComponent extends UAgentComponent {}

// =============================================================================
// -- Game ---------------------------------------------------------------------
// =============================================================================

/**
 * @has - - - FDataRouter
 */
class ICarlaGameControllerBase {}

/**
 * @composed - - - ICarlaGameControllerBase
 * @composed - - - FDataRouter
 * @composed - - - UCarlaSettings
 */
class UCarlaGameInstance extends UGameInstance {}

 // * @has - Player - ACarlaVehicleController

/**
 * @depend - - - UCarlaGameInstance
 * @composed - - - UTaggerDelegate
 * @composed - - - ADynamicWeather
 * @composed - - - AVehicleSpawnerBase
 * @composed - - - AWalkerSpawnerBase
 */
class ACarlaGameModeBase extends AGameModeBase {}

class ACarlaHUD extends AHUD {}

class ACarlaPlayerState extends APlayerState {}

/**
 * @depend - - - UAgentComponent
 * @depend - - - ACarlaVehicleController
 * @composed - - - ISensorDataSink
 */
class FDataRouter {}

/**
 * @composed - - - FMockSensorDataSink
 */
class MockGameController extends ICarlaGameControllerBase {}

class FMockSensorDataSink extends ISensorDataSink {}

/**
 * @depend - - - ATagger
 */
class UTaggerDelegate extends UObject {}

class ATagger /*extends AActor*/ {}

// =============================================================================
// -- MapGen -------------------------------------------------------------------
// =============================================================================

// MapGen is ignored here.

// =============================================================================
// -- Sensor -------------------------------------------------------------------
// =============================================================================

class ALidar extends ASensor {}

class ASceneCaptureCamera extends ASensor {}

/**
 * @depend - - - ISensorDataSink
 */
class ASensor extends AActor {}

class ISensorDataSink {}

// =============================================================================
// -- Server -------------------------------------------------------------------
// =============================================================================

/**
 * @depend - - - CarlaServerAPI
 */
class FCarlaServer {}

/**
 * @composed - - - FCarlaServer
 * @has - - - FServerSensorDataSink
 */
class FServerGameController extends ICarlaGameControllerBase {}

/**
 * @depend - - - FCarlaServer
 */
class FServerSensorDataSink extends ISensorDataSink {}

/**
 * CarlaServer
 * Library API
 * @opt commentname
 * @opt nodefillcolor #fdf6e3
 */
class CarlaServerAPI {}

// =============================================================================
// -- Settings -----------------------------------------------------------------
// =============================================================================

class UCameraDescription extends USensorDescription {}

/**
 * @composed - - - USensorDescription
 */
class UCarlaSettings extends UObject {}

class ULidarDescription extends USensorDescription {}

class USensorDescription extends UObject {}

// =============================================================================
// -- Traffic ------------------------------------------------------------------
// =============================================================================

class ATrafficLightBase extends ATrafficSignBase {}

/**
 * @composed - - - UTrafficSignAgentComponent
 */
class ATrafficSignBase extends AActor {}

// =============================================================================
// -- Util ---------------------------------------------------------------------
// =============================================================================

class AActorWithRandomEngine extends AActor {}

// =============================================================================
// -- Vehicle ------------------------------------------------------------------
// =============================================================================

/**
 * @has - - - ACarlaPlayerState
 * @has - - - ACarlaHUD
 */
class ACarlaVehicleController extends AWheeledVehicleController {}

/**
 * @composed - - - UVehicleAgentComponent
 */
class ACarlaWheeledVehicle extends AWheeledVehicle {}

/**
 * @has - - - ACarlaWheeledVehicle
 */
class AVehicleSpawnerBase extends AActorWithRandomEngine {}

/**
 * @has - - - ACarlaWheeledVehicle
 */
class AWheeledVehicleAIController extends APlayerController {}

/**
 * @composed - - - UCameraComponent
 */
class AWheeledVehicleController extends AWheeledVehicleAIController {}

// =============================================================================
// -- Walker -------------------------------------------------------------------
// =============================================================================

/**
 * @has - - - ACharacter
 */
class AWalkerAIController extends AAIController {}

/**
 * @has - - - ACharacter
 */
class AWalkerSpawnerBase extends AActorWithRandomEngine {}

// =============================================================================
// -- Blueprints ---------------------------------------------------------------
// =============================================================================

/**
 * Carla Game Mode
 * @opt commentname
 * @opt nodefillcolor #bfe4ff
 */
class CarlaGameMode extends ACarlaGameModeBase {}

/**
 * Speed Limit Sign
 * @opt commentname
 * @opt nodefillcolor #bfe4ff
 */
class SpeedLimitSignBlueprint extends ATrafficSignBase {}

/**
 * Traffic Light
 * @opt commentname
 * @opt nodefillcolor #bfe4ff
 */
class TrafficLightBlueprint extends ATrafficLightBase {}

/**
 * Vehicle Blueprints
 * @opt commentname
 * @opt nodefillcolor #bfe4ff
 */
class VehicleBlueprints extends ACarlaWheeledVehicle {}

/**
 * Vehicle Spawner
 * @opt commentname
 * @opt nodefillcolor #bfe4ff
 */
class VehicleSpawnerBlueprint extends AVehicleSpawnerBase {}

/**
 * Walker Blueprints
 * @opt commentname
 * @opt nodefillcolor #bfe4ff
 * @composed - - - UWalkerAgentComponent
 */
class WalkerBlueprints extends ACharacter {}

/**
 * Walker Spawner
 * @opt commentname
 * @opt nodefillcolor #bfe4ff
 */
class WalkerSpawnerBlueprint extends AWalkerSpawnerBase {}
