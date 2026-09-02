# Sesión: Light Defaults tool (CarlaUE5, rama marcos/ue58_Lights)

Última actualización: 2026-09-01. Contexto para retomar mañana.

## Estado de commits

**Fase Weather/PostProcess: cerrada y ya committeada** (ver
`SESSION_NOTES_weather_postprocess.md`, histórico, no tocar).

**Fase actual (Light Defaults tool) — SIGUE SIN COMMITEAR, nunca se ha
pedido.**

`git status` repo código (`/home/adas/CarlaUE5`), snapshot 2026-09-01:
```
M  Plugins/Carla/Source/Carla/Carla.Build.cs
M  Plugins/Carla/Source/Carla/Carla.cpp
M  Plugins/Carla/Source/Carla/Lights/CarlaLight.cpp
M  Plugins/Carla/Source/Carla/Lights/CarlaLight.h
M  Plugins/Carla/Source/Carla/Sensor/CustomV2XSensor.cpp   (fix de un include que faltaba, no relacionado con Light Defaults pero necesario para compilar)
M  Plugins/Carla/Source/Carla/Vehicle/CarlaWheeledVehicle.cpp   (integración runtime con manual_control, ver sección 4)
M  Plugins/Carla/Source/Carla/Vehicle/CarlaWheeledVehicle.h
M  Plugins/Carla/Source/Carla/Weather/Sky.cpp    (Moon ForwardShadingPriority/AtmosphereSunLight, ver más abajo)
M  Plugins/Carla/Source/Carla/Weather/Weather.cpp
?? Plugins/Carla/Source/Carla/BlueprintLibary/LightDefaultsJsonUtils.cpp
?? Plugins/Carla/Source/Carla/BlueprintLibary/LightDefaultsJsonUtils.h
?? Plugins/Carla/Source/Carla/Lights/LightDefaultsEditorPanel.cpp
?? Plugins/Carla/Source/Carla/Lights/LightDefaultsEditorPanel.h
```

`git status` repo Content (`Unreal/CarlaUnreal/Content/Carla`), snapshot 2026-09-01 — **crecido mucho
respecto a la última nota, revisar antes de commitear**:
```
M  Blueprints/Lights/BP_Lights.uasset
M  Maps/Town10HD_Opt.umap
M  Static/Building/BackgroundBuildings/00_IndividualComponents/Apt12/MI_Apt12_FakeInterior.uasset
M  Static/Building/BackgroundBuildings/00_IndividualComponents/Apt20/MI_Apt20.uasset
M  Static/Building/BackgroundBuildings/00_IndividualComponents/Apt20/MI_Apt20_withEmissive.uasset
M  Static/Building/BackgroundBuildings/00_IndividualComponents/Apt20/MI_Apt21.uasset
M  Static/Building/BackgroundBuildings/BP_Apt20_A.uasset
M  Static/Building/Buildings/BuildingPieces/00_IndividualComponents/Shop02/MI_Shop02_Btt_Details_{A,B,C}.uasset
M  Static/Building/Buildings/BuildingPieces/00_IndividualComponents/Skysc22/MI_Skysc22_Btt_Interior.uasset
M  Static/Building/Buildings/BuildingPieces/00_IndividualComponents/Skysc23/MI_Skysc23_Btt_DoorCorner.uasset
M  Static/Building/SpecialBuildings/Factory/IndividualComponents/MI_FactoryLights.uasset
M  Static/Building/SpecialBuildings/Museum/IndividualComponents/MI_FocusLight.uasset
M  Static/Building/SpecialBuildings/Museum/IndividualComponents/MI_Museum.uasset
M  Static/Building/SpecialBuildings/Museum/IndividualComponents/MI_MuseumGate.uasset
M  Static/Building/SpecialBuildings/Museum/IndividualComponents/M_Focus.uasset
M  Static/Building/SpecialBuildings/Parking/IndividualComponents/MI_ParkingRamp.uasset
M  Static/Bus/MitsubishiFusoRosa/Materials/MI_MitsubishiFusoRosa_Emissive.uasset
M  Static/Dynamic/Street/Materials/MI_FoodCart_Body.uasset
M  Static/GenericMaterials/000_Masters/Buildings/M_FakeInterior.uasset
M  Static/GenericMaterials/00_LegacyMaterials/Building/M_Building_Master.uasset
M  Static/Static/00_LegacyAssets/Materials/Billboard/M_Building_Master_Inst.uasset
M  Static/Static/00_LegacyAssets/Materials/chain_barrier/M_LightPost.uasset
M  Static/Static/Materials/BusStop/MI_BusStop.uasset
M  Static/StreetLight/BP_StreetLight06_A.uasset
M  Static/Truck/ActrosFiretruck/Materials/MI_ActrosFiretruck_Emissive.uasset
?? Config/Lights/   (Defaults.json)
?? Maps/Town10HD_Opt_BuiltData.uasset
```
La mayoría de los `MI_*_Emissive.uasset`/`MI_*` nuevos respecto a ayer son
efecto colateral de abrir/tocar esos assets vía escaneos headless de la
sesión (building families, vehicle catalog) — no se ha guardado nada a
propósito salvo `M_FakeInterior` (grafo, sesión anterior) y `Weather.cpp`/
`Sky.cpp` en código. **Revisar con `git diff --stat` antes de plantear
ningún commit** — no asumir que todo es intencional.

## Objetivo

Tool en el editor (`Window → Light Defaults`) para editar color/intensity/
emissive de luces y ventanas de edificios sin abrir cada Blueprint/material
uno a uno. Catálogo JSON reusable entre mapas
(`Content/Carla/Config/Lights/Defaults.json`), keyed por clase de asset con
fallback por `LightGroup` — no por mapa.

## Arquitectura actual (tres sistemas independientes, una sola tool)

### 1. Luces `UCarlaLight` (farolas, semáforos-BP, vehículos, "Other")
- Fuente: `UCarlaLightSubsystem::GetLights()` del mundo del editor.
- Dedup por **clase del actor dueño** (`BP_StreetLight01_C`, etc.), no por
  la clase del componente `BP_Lights_C` (que es igual en todas las
  instancias).
- Aplicación 100% nativa en C++ (`CarlaLight.cpp`): `ApplyIntensityToComponents`/
  `ApplyColorToComponents`/`ApplyEmissiveToComponents`. El chequeo `UpdateLights`
  (BlueprintImplementableEvent en `BP_Lights`) está **confirmado muerto** —
  nunca tocaba el componente nativo, probado con log instrumentado con un
  valor centinela (12345). Se bypasea del todo.
- Emissive: el Lerp del material está gateado por un scalar "On/Off" que por
  defecto vale 0 — hay que fijar `EmissiveIntensity` Y `On/Off` juntos o no
  se ve nada.
- Tabs: Street / Building (grupo `UCarlaLight`, no confundir con la pestaña
  nueva "Buildings" de ventanas) / Vehicle / Other.

### 2. Semáforos (`ATrafficLightBase`)
- No tienen `UCarlaLight`. Se escanean con `TActorIterator<ATrafficLightBase>`.
- Solo se controla el "Emissive" (mapea a `lightvalue` del material del
  cristal de la señal) + un toggle "Preview On" (fuerza `On/Off` del
  material sin tocar JSON, para ver el color sin darle a Play).
- **Nunca se crea un MID nuevo** para semáforos — solo se escribe sobre el
  MID que `BP_TLOpenDrive_RHT/LHT` ya creó en su Construction Script y cuyo
  array (`RedLights`/`YellowLights`/`GreenLights`) sigue referenciando por
  puntero para su ciclo rojo/ámbar/verde. Crear uno nuevo desincronizaría el
  render del array del BP y rompería el ciclo.
- Confirmado funcionando por el usuario.
- Gap conocido, aceptado, sin resolver: algunos `BP_TrafficLightNew_T10_master_*`
  spawean una farola decorativa vía un array `Props` en su Construction
  Script, sin ningún componente de luz — no hay solución rápida, el usuario
  decidió pasar a otra cosa.

### 3. Edificios bakeados ("Buildings" tab) — lo grueso de hoy
- El "bake tool" del proyecto (`BP_BuildingConverterToCode`) genera actores
  **nativos `AActor` puros, sin subclase Blueprint** — confirmado con script
  headless: 130 actores en Town10HD_Opt de clase literal `Actor`
  (`GetClass()->GetName() == "Actor"`), labels tipo `PB_Skyscrapper02_01`.
  No hay clase a la que dedupear como en los otros grupos → **una única fila
  sintética "ProceduralBuildings"** cubre todos los edificios del nivel.
- Detección de qué tocar: cualquier slot de `UStaticMeshComponent` (incluye
  `HierarchicalInstancedStaticMeshComponent`, que hereda de él) cuyo asset de
  material estático cumpla: vive bajo `/Game/Carla/Static/Building/` Y su
  nombre contiene `"FakeInterior"` Y expone el parámetro `EmissiveIntensity`.
  - **Ojo con el material base real**: es `M_FakeInterior`
    (`/Game/Carla/Static/GenericMaterials/000_Masters/Buildings/M_FakeInterior`),
    confirmado con 34 Material Instances usándolo, todas bajo contenido de
    edificios — NO `M_MaterialMaster` (el master genérico compartido por
    todo el proyecto, calles/coches/props). Primer intento de esta sesión
    asumió mal que era `M_MaterialMaster` por haber testeado sin querer
    otras piezas del edificio (paredes/esquinas), no el FakeInterior en sí.
    El usuario pilló el error preguntando "seguro que es M_MaterialMaster y
    no M_FakeInterior?" — merecía la duda.
  - Bug de re-detección: tras el primer `CreateAndSetMaterialInstanceDynamic`,
    el slot pasa a ser un MID transitorio cuyo propio `GetPathName()` ya no
    vive bajo `/Game/...` → el filtro por path dejaba de reconocerlo en
    escaneos/selects/edits posteriores. Fix: si el material es un
    `UMaterialInstanceDynamic`, comprobar `MID->Parent` (el asset estático
    original) en vez del propio MID.
- **Granularidad real y por qué el % no controlaba ventana a ventana**:
  confirmado con script headless (`NumCustomDataFloats == 0` en todos los
  HISM) que no hay datos per-instancia horneados. Un único componente HISM
  puede dibujar **hasta 1008 instancias físicas** compartiendo un solo slot
  de material — controlar el MID de ese slot enciende/apaga el bloque
  entero a la vez, nunca una ventana individual.
- **Solución final**: se editó el grafo de `M_FakeInterior` (vía
  `unreal.MaterialEditingLibrary` headless) añadiendo:
  - Nodo `MaterialExpressionPerInstanceRandom` (valor aleatorio estable por
    instancia física, resuelto por el renderer, sin necesidad de
    `NumCustomDataFloats`).
  - Nuevo parámetro escalar `PercentLitThreshold` (default `1.0`, para que
    cualquier MI existente que la tool no toque se comporte exactamente
    igual que antes).
  - Nodo `MaterialExpressionIf`: `Gate = (PerInstanceRandom < PercentLitThreshold) ? 1 : 0`.
  - Ese `Gate` se multiplica contra la salida que ya alimentaba
    `EmissiveColor` (`MaterialExpressionMultiply_1` original), sin tocar
    nada del cableado interno existente — solo se intercepta la conexión
    final justo antes del output del material.
  - Recompilado y guardado (`recompile_material` + `save_loaded_asset`).
  - C++ (`ApplyToActorSlots`) ahora solo fija `EmissiveIntensity`,
    `On/Off=1` y `PercentLitThreshold = PercentLit/100` sobre cada MID — ya
    no hay ningún `FRandomStream`/hash en el lado C++, todo el
    "quién se enciende" lo decide el shader por instancia real.
  - **Confirmado por el usuario: "funciona perfecto"** — variación ahora es
    por ventana física individual, no por bloque de componente HISM.
- Se intentó antes una distinción "pareja on/off horneada en el asset"
  (`DefaultOnOff` del material original) para no encender variantes ya
  autoradas como apagadas — **se abandonó**: la mayoría de instancias
  `FakeInterior` del proyecto están horneadas en off por defecto, así que
  respetar eso dejaba casi todo incontrolable (solo se encendía 1 módulo en
  todo Town10). El enfoque final (`PerInstanceRandom` en material) sustituye
  esa idea por completo; ya no se distingue variante on/off horneada, cada
  ventana se decide solo por el nuevo `PercentLitThreshold`.
- Fila "Buildings" en la UI: columna "Intensity" reetiquetada (solo vía
  tooltip, el header de columna sigue diciendo "Intensity" globalmente) como
  "Percent Lit" — reusa el campo `Current.Intensity` (0-100) de
  `FLightAssetDefault` en vez de crear un campo nuevo en el JSON. Columna
  "Color" oculta para esta fila (no aplica). "Select" selecciona los ~130
  actores con al menos un slot `FakeInterior` matched.
- Deferred/aceptado, sin tocar: static meshes puestos por `BP_Spline`
  (farolas/pivotes) sin `BP_Lights` — usuario aceptó "emisivo siempre
  encendido" como workaround, no se ha tocado código.

### 4. Vehículos (pestaña "Vehicles") — grueso de la sesión 2026-09-01
- Catálogo completo desde `VehicleParameters.json` (24 vehículos con
  `HasLights`), no solo los ya colocados en el nivel. `ScanVehicleCatalog`
  spawnea cada clase temporalmente muy por debajo del mapa
  (`FVector(0,0,-1000000)`) e inspecciona luces reales + slots emisivos,
  luego destruye — la CDO sola no sirve, el Construction Script (que monta
  mallas/materiales) nunca corre sobre ella.
- **Semántica confirmada por el usuario (al revés de mi primer intento)**:
  el campo global "Intensity" de la fila es el valor que se escribe en
  **todos** los parámetros escalares emisivos expuestos por el material
  (`Position`/`Low Beam`/`High Beam`/.../`Special1`, todos a la vez). Las
  columnas por grupo (Position, Low Beam, ...) son la **Intensity real** de
  los componentes Spot/Rect/Point del BP, no el material.
- Detección de materiales emisivos por **ancestría base real**
  (`ResolveVehicleMaterialBase` sube la cadena de padres de la MI hasta el
  `UMaterial` final y comprueba `Contains("VehicleLightsMaster")`), no por
  substring `"Emissive"` en el nombre — el substring fallaba (falsos
  negativos en algunas mallas, falsos positivos en `Glass_Emissive` sin
  relación).
- Escala fija `x10000` (`VehicleLightIntensityDisplayScale`) para mostrar en
  UI los valores reales de Intensity de luz (que rondan cientos de miles) en
  números manejables. El usuario aceptó explícitamente que sea fija/global
  aunque dé resultados no uniformes entre vehículos, en vez de un valor por
  vehículo.
- Botón "Preview" (por fila) sustituye a "Select": spawnea el vehículo,
  hace raycast y reencuadra la cámara del viewport del editor. Al activarlo
  también fuerza `VisualTime=0.25` del MPC `CarlaParameters` (ver blinkers
  abajo) para que los intermitentes se vean encendidos en el preview sin
  necesidad de un botón aparte (existió un botón suelto "Preview Blinkers";
  se quitó y se fusionó dentro de `ToggleVehiclePreview`).
- Valores por defecto de Ambulance/DodgeCharger y Crossbike/RoadBike
  copiados en bloque al resto de coches/bicis respectivamente
  (`Defaults.json → perVehicleClass`), a petición explícita del usuario
  señalando valores concretos en capturas de pantalla.

#### Intermitentes (blinkers) — gating por Sine('VisualTime')
- El brillo del emisivo de los intermitentes está gateado en el material por
  un nodo `Sine` sobre el parámetro escalar `VisualTime` del Material
  Parameter Collection `/Game/Carla/Blueprints/Game/CarlaParameters`. Ese
  parámetro solo avanza durante Play — en el viewport del editor en pausa
  puede caer en la mitad oscura de la onda, pareciendo "roto" sin estarlo.
  Confirmado por el usuario probando manualmente valores 0.1–0.4 / 1.1–1.4.
- **RESUELTO (2026-09-02)**: el desfase entre el parpadeo del emisivo
  (cuerpo del vehículo) y el de la luz real (iluminación proyectada sobre
  carretera/entorno) no era un timer de Blueprint (esa vía se investigó a
  fondo y se descartó — `LightsUpdater`/`SetAllLightState`/
  `SetLightListState` solo hacen `Set Visibility`/`Set Scalar Parameter`
  estáticos por cambio de estado, sin ninguna oscilación; el `SetTimer`
  encontrado en `BP_Mustang`/`BP_Tesla`/`BP_Audi_Etron` está confirmado
  muerto — nadie lo llama). La causa real: cada light component físico del
  blinker (`front-blinker-l-1`, etc.) tiene asignado un **Light Function
  Material**, `/Game/Carla/Static/GenericMaterials/000_Masters/Vehicles/
  LFM_Indicator` — modula la luz proyectada sin tocar `Intensity`/
  `Visibility` del componente, por eso era invisible a cualquier grep de
  C++/Blueprint. Ese material leía el nodo nativo `Time` (tiempo del motor/
  nivel, sin reset) en vez del `VisualTime` de la MPC `CarlaParameters`
  (que sí resetea a 0 en el `BeginPlay` del episodio) — cualquier rato
  entre cargar el mapa y pulsar Play dejaba un offset de fase permanente
  entre ambos parpadeos. Fix: sustituido el nodo `Time` por un
  `CollectionParameter` (`CarlaParameters.VisualTime`), mismo `Period=1`
  que ya tenía el `Sine`. Editado a mano por el usuario en el grafo de
  `LFM_Indicator`. **Confirmado por el usuario: "funciona".**

#### Integración runtime con manual_control.py / generate_traffic.py
- **Bug detectado y corregido esta sesión**: mi primer intento llamaba
  `ApplyVehicleLightsToSingleActor` (la función "todo a la vez", pensada
  para el Preview del editor) sin condición desde
  `ACarlaWheeledVehicle::ActivateVehicleLightComponents()` (`BeginPlay`) —
  esto encendía **todos** los grupos emisivos al spawnear, sin mirar el
  estado real de luces, y además `RefreshLightState` (evento Blueprint puro,
  opaco a C++, ver abajo) repisaba valores propios débiles al pulsar 'L'.
- **Causa raíz**: `RefreshLightState` (`BlueprintImplementableEvent`,
  llamado desde `SetVehicleLightState` en cada cambio de `FVehicleLightState`)
  vive enteramente en el grafo Blueprint de cada vehículo — no hay forma de
  inspeccionar ni interceptar su lógica interna desde C++. Confirmado con un
  log temporal (`TFieldIterator<FFloatProperty>` sobre la clase) que solo
  expone DOS variables float propias del Blueprint: `PositionLights` y
  `Beam Lights` (usadas para escalar la Intensity real de esos dos grupos en
  el Construction Script, no para el material) — el resto de grupos
  (Blinkers/Brake/Reverse/Fog/Special1) no tienen variable expuesta
  equivalente, su valor vive hardcodeado dentro del grafo de
  `RefreshLightState`.
- **Fix**: en vez de intentar interceptar/sobreescribir variables del
  Blueprint, se añadió `ULightDefaultsJsonUtils::ApplyVehicleLightsRuntimeState`
  (distinta de `ApplyVehicleLightsToSingleActor`/`ApplyVehicleLightsLive`,
  que siguen siendo solo para el Preview del editor) que:
  - Pone el parámetro material de cada grupo a `Intensity` **solo si ese
    grupo está ON** en el `FVehicleLightState` actual, `0` si no — nunca
    todos a la vez.
  - Aplica la Intensity real de luz por grupo sin condición (el propio
    Active/Visibility del componente ya gatea el render — coherente con que
    esto "ya funcionaba" según el usuario para todos los grupos salvo
    blinkers).
  - Se llama **dos veces**: una en `BeginPlay` (con el estado inicial, casi
    siempre todo apagado) y otra **justo después de** `RefreshLightState(...)`
    dentro de `SetVehicleLightState`, para que nuestro valor gane siempre
    sobre lo que el grafo Blueprint acabe de repisar.
  - El default guardado en JSON se cachea una vez por vehículo
    (`bHasSavedVehicleLightDefault`/`SavedVehicleLightIntensity`/
    `SavedVehicleLightGroupIntensity` en `CarlaWheeledVehicle.h`) para no
    releer disco en cada toggle de luces.
- **Verificado headless** (servidor standalone `-game -nullrhi`, spawn de
  `BP_LincolnMKZ` vía PythonAPI + `set_light_state(LowBeam|LeftBlinker)`,
  log temporal `DEBUGRUNTIME`): en spawn, los 9 grupos material a `0.0`; tras
  togglear Low Beam + Left Blinker, esos dos a `100000` (el `Intensity`
  guardado en `Defaults.json` para esa clase) y el resto siguen en `0.0`.
  Log temporal ya retirado del código tras confirmar.
- **Confirmado por el usuario en editor real: "funciona todo"** — salvo el
  desfase de parpadeo de intermitentes (ver arriba).

## UX de la tool (rediseño pedido explícitamente por el usuario)

- **Live-apply-on-edit**: cada edición de campo en una fila se aplica
  inmediatamente al nivel (`ApplyRowLive`/`ApplyTrafficRowLive`/
  `ApplyBuildingRowLive`) — **no** escribe en `Defaults.json`.
- **Save All**: único botón que comitea `Current` de **todas** las filas
  escaneadas (de las tres pestañas: `AllRows`, `AllTrafficRows`,
  `AllBuildingRows`, no solo la vista filtrada) a disco.
- **Reapply To Level**: revierte cualquier edición live no guardada, releyendo
  lo que hay en disco (`ReapplyLightDefaultsToLevel` +
  `ReapplyTrafficLightDefaultsToLevel` + `ReapplyProceduralBuildingsToLevel`).
- **Select** por fila: selecciona/deselecciona en el Outliner los actores de
  esa clase (o, para Buildings, todos los actores con match).
- Hook `FEditorDelegates::OnMapOpened` + delay de 1s (necesario, un solo tick
  no basta — confirmado por el usuario con "mismo comportamiento" tras
  probar solo `SetTimerForNextTick`) reaplica los tres catálogos al abrir un
  mapa. La causa raíz: el `RegisterLight()` de cada luz aplica su emissive
  demasiado pronto, antes de que asiente el streaming de World Partition, y
  algo posterior lo resetea silenciosamente.

## Verificado esta sesión

- **`BP_CarlaCola` (Vehicles tab)**: reportado como "sin luces". Investigado
  a fondo (nombres de componente, ancestría de material emisivo hasta
  `M_VehicleLightsMaster`) — todo correcto salvo un gap real menor y ya
  conocido: CarlaCola no tiene `back-position-*` (solo delantera, a
  diferencia de `BP_Ambulance` que sí tiene trasera). **Causa real del
  reporte, distinta**: el botón **Preview** de esa fila spawnea el actor
  equivocado — una máquina de bebidas ("máquina de bebida"), no el camión —
  así que el usuario veía en pantalla un prop sin luces y asumía que los
  valores de la fila estaban rotos. Los valores de intensidad en sí
  funcionan bien una vez aplicados sin depender del Preview (confirmado por
  el usuario poniendo valores a mano e ignorando el Preview). **Bug de
  Preview para CarlaCola sigue sin arreglar** — no investigada la causa
  (candidato: resolución de clase/raycast de encuadre de cámara del botón
  Preview cogiendo el prop equivocado cerca del punto de spawn muy por
  debajo del mapa, o algo en `ToggleVehiclePreview` específico de esta
  clase). Ver "Pendiente" abajo.
- Reapply To Level ya no crashea y sí aplica visualmente (root cause: cadena
  `UpdateLights` muerta, bypaseada con C++ nativo).
- Point/Spot/Rect lights: confirmado que ya se cubren todos (`Spot`/`Rect`
  heredan de `UPointLightComponent`, `GetComponents<UPointLightComponent>()`
  ya los incluye).
- Semáforos: emissive + preview on/off funcionando.
- Edificios: Percent Lit funcionando ventana a ventana tras el fix de
  material — **confirmado por el usuario, "funciona perfecto".**

## Pendiente / próximos pasos

1. ~~Sincronizar parpadeo emisivo/luz real de intermitentes~~ — **RESUELTO
   2026-09-02**, ver sección 4 arriba (`LFM_Indicator` leía `Time` nativo en
   vez de `VisualTime` de la MPC).
2. Decidir si se commitea algo de esto (nunca pedido explícitamente aún) —
   revisar antes con `git diff --stat` la lista larga de `.uasset`/`.umap`
   de origen dudoso en el repo Content (ver "Ojo" arriba, ha crecido
   respecto a ayer).
3. Cosmético: el header de columna sigue diciendo "Intensity" también en la
   pestaña Buildings (donde en realidad es "Percent Lit") — solo hay
   tooltip por fila, no cambia el texto del header según la pestaña activa.
4. Revisar si "Select" en Buildings (selecciona ~130 actores de golpe) es
   útil tal cual o conviene acotarlo.
4b. Bug sin arreglar: botón **Preview** en la fila `CarlaCola` (Vehicles
   tab) spawnea una máquina de bebidas en vez del camión — sin investigar
   aún, ver "Verificado esta sesión" arriba. Workaround actual: ignorar
   Preview para esa fila, ajustar valores a mano.
5. Gaps conocidos y aceptados, no urgentes: BP_Spline sin `BP_Lights`,
   farola decorativa embebida en `BP_TrafficLightNew_T10_master_*` sin
   componente de luz.
6. No probado: comportamiento con varios niveles/sublevels de World
   Partition abiertos a la vez (solo probado con Town10HD_Opt como único
   nivel).

## Notas de tooling para la próxima sesión

- **Compilar**: `/home/adas/UnrealEngine_5.5/Engine/Build/BatchFiles/Linux/Build.sh CarlaUnrealEditor Linux Development -waitmutex -project="/home/adas/CarlaUE5/Unreal/CarlaUnreal/CarlaUnreal.uproject"`.
  **Editor cerrado antes de compilar** (`pgrep -fl UnrealEditor | grep -v zsh`
  debe salir vacío) — recompilar con UFUNCTIONs/clases nuevas con el editor
  vivo puede romper el hot reload. **Ojo:** en esta sesión `pgrep` dio vacío
  una vez estando el editor realmente abierto (según capturas del usuario) —
  ante la duda, preguntar directamente en vez de confiar ciegamente en el
  pgrep.
- **Headless Python**: `UnrealEditor-Cmd <uproject absoluta> -stdout -unattended -nosplash -nopause -nullrhi -EnablePlugins=PythonScriptPlugin -ExecutePythonScript=<script.py>`.
  `print()` no aparece en el log — usar siempre `unreal.log_warning(...)`.
  `unreal.EditorAssetLibrary.load_asset` falla con paths válidos — usar
  `unreal.load_asset("<PackagePath>.<AssetName>")` (path de objeto completo).
- **Editar grafos de material headless** (`unreal.MaterialEditingLibrary`):
  - `create_material_expression(material, class, x, y)`,
    `connect_material_expressions(from_expr, from_output, to_expr, to_input)`,
    `connect_material_property(expr, output, MaterialProperty.MP_X)`,
    `get_material_property_input_node`/`_output_name` (para leer qué
    alimenta un output antes de tocar nada),
    `recompile_material(mat)`, luego `unreal.EditorAssetLibrary.save_loaded_asset(mat)`.
  - Nombres de pines de `MaterialExpressionIf` son `"A"`, `"B"`,
    `"A > B"`, `"A == B"`, `"A < B"` — NO `"AGreaterThanB"` etc (falló al
    primer intento, se descubrió con `get_material_expression_input_names`).
  - Patrón seguro: acumular el resultado booleano de cada
    `connect_material_expressions` y **abortar sin guardar** si alguno
    falló, en vez de guardar un grafo a medio cablear. Cada ejecución del
    script recrea nodos nuevos (no reusa los de un intento fallido anterior)
    pero como nunca se llegó a guardar, no hay basura persistida en disco.
  - `unreal.EditorAssetLibrary.load_asset` para leer, pero para inspeccionar
    parámetros/valores por defecto usar `unreal.MaterialEditingLibrary.
    get_scalar_parameter_names` / `get_material_instance_scalar_parameter_value`
    (esta última falla sobre un `Material` base, solo funciona sobre
    `MaterialInstanceConstant`).
- Nunca ejecutar `UnrealEditor-Cmd` headless con el editor GUI del usuario
  abierto sobre el mismo proyecto (riesgo de lock de asset registry/DDC) —
  comprobar `pgrep` y/o preguntar antes.
