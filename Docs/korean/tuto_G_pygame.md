# 차량 제어를 위한 Pygame

[__PyGame__](https://www.pygame.org/news)은 비디오 게임 작성에 유용한 크로스 플랫폼 Python 모듈 세트입니다. 카메라와 같은 센서 출력을 모니터링하기 위해 CARLA의 실시간 시각적 출력을 렌더링하는 유용한 방법을 제공합니다. PyGame은 키보드 이벤트도 캡처할 수 있으므로 차량과 같은 액터를 제어하는 좋은 방법입니다.

이 튜토리얼에서는 Traffic Manager(TM)가 제어하는 자율 주행 교통을 모니터링하고 키보드를 사용하여 모든 차량을 수동으로 제어할 수 있는 간단한 PyGame 인터페이스를 설정하는 방법을 배웁니다.

## 시뮬레이터 설정 및 교통 관리자 초기화

먼저 TM을 초기화하고 도시 주변에 무작위로 분포된 교통을 생성하겠습니다.

```py
import carla
import random
import pygame
import numpy as np

# 클라이언트에 연결하고 월드 객체 검색
client = carla.Client('localhost', 2000)
world = client.get_world()

# 동기 모드로 시뮬레이터 설정
settings = world.get_settings()
settings.synchronous_mode = True # 동기 모드 활성화
settings.fixed_delta_seconds = 0.05
world.apply_settings(settings)

# 동기 모드로 TM 설정
traffic_manager = client.get_trafficmanager()
traffic_manager.set_synchronous_mode(True)

# 필요한 경우 동작을 반복할 수 있도록 시드 설정
traffic_manager.set_random_device_seed(0)
random.seed(0)

# 우리가 하는 일을 볼 수 있도록 관전자도 설정할 것입니다
spectator = world.get_spectator()
```

## 차량 생성

도시 전체에 생성된 차량들의 모음을 만들고 TM이 이들을 제어하도록 하고자 합니다.

```py
# 맵의 생성 지점 검색
spawn_points = world.get_map().get_spawn_points()

# 블루프린트 라이브러리에서 일부 모델 선택
models = ['dodge', 'audi', 'model3', 'mini', 'mustang', 'lincoln', 'prius', 'nissan', 'crown', 'impala']
blueprints = []
for vehicle in world.get_blueprint_library().filter('*vehicle*'):
    if any(model in vehicle.id for model in models):
        blueprints.append(vehicle)

# 최대 차량 수를 설정하고 생성할 차량 목록 준비
max_vehicles = 50
max_vehicles = min([max_vehicles, len(spawn_points)])
vehicles = []

# 생성 지점의 무작위 샘플을 선택하고 일부 차량 생성
for i, spawn_point in enumerate(random.sample(spawn_points, max_vehicles)):
    temp = world.try_spawn_actor(random.choice(blueprints), spawn_point)
    if temp is not None:
        vehicles.append(temp)

# 생성된 차량 목록을 구문 분석하고 set_autopilot()을 통해 TM에 제어권 부여
for vehicle in vehicles:
    vehicle.set_autopilot(True)
    # 차량이 신호등을 무시할 확률을 무작위로 설정
    traffic_manager.ignore_lights_percentage(vehicle, random.randint(0,50))
```

## PyGame으로 카메라 출력 렌더링 및 차량 제어

이제 교통으로 가득 찬 도시가 있으므로, 차량 중 하나를 따라가는 카메라를 설정하고 키보드 입력으로 이를 제어할 수 있는 제어 인터페이스를 설정할 수 있습니다.

먼저, 픽셀 데이터를 PyGame 인터페이스에 렌더링하기 위한 `camera.listen(...)` 콜백 함수를 정의해야 합니다. PyGame은 데이터를 표면에서 화면으로 렌더링하므로, 콜백에서는 콜백 함수에 전달된 객체에 저장된 표면을 채웁니다.

```py
# PyGame 표면을 유지하고 전달하기 위한 렌더링 객체
class RenderObject(object):
    def __init__(self, width, height):
        init_image = np.random.randint(0,255,(height,width,3),dtype='uint8')
        self.surface = pygame.surfarray.make_surface(init_image.swapaxes(0,1))

# 카메라 센서 콜백, 카메라의 원시 데이터를 2D RGB로 재구성하여 PyGame 표면에 적용
def pygame_callback(data, obj):
    img = np.reshape(np.copy(data.raw_data), (data.height, data.width, 4))
    img = img[:,:,:3]
    img = img[:, :, ::-1]
    obj.surface = pygame.surfarray.make_surface(img.swapaxes(0,1))
```

이제 제어 로직을 처리할 객체를 만들 것입니다. 이는 종종 특정 요구 사항에 맞게 조정이 필요할 수 있지만, 여기서는 기본적인 인터페이스를 설명합니다. 차량을 제어할 때, 이 제어 인터페이스는 표준 키보드의 화살표 키를 통한 제어를 허용합니다. 위쪽 화살표는 가속, 아래쪽 화살표는 제동, 왼쪽과 오른쪽 화살표는 차량을 회전시킵니다. 차량이 정지해 있거나 정지할 때 아래쪽 화살표를 누르고 있으면 후진 기어가 작동하여 뒤로 움직이기 시작합니다.

```py
# 차량 제어를 관리하는 제어 객체
class ControlObject(object):
    def __init__(self, veh):
        
        # 제어 상태를 저장하는 제어 매개변수
        self._vehicle = veh
        self._steer = 0
        self._throttle = False
        self._brake = False
        self._steer = None
        self._steer_cache = 0
        # 차량의 제어 상태를 변경하기 위해 
        # carla.VehicleControl 객체가 필요합니다
        self._control = carla.VehicleControl()
    
    # PyGame 창에서 키 누름 이벤트를 확인하고
    # 제어 상태를 정의합니다
    def parse_control(self, event):
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_RETURN:
                self._vehicle.set_autopilot(False)
            if event.key == pygame.K_UP:
                self._throttle = True
            if event.key == pygame.K_DOWN:
                self._brake = True
            if event.key == pygame.K_RIGHT:
                self._steer = 1
            if event.key == pygame.K_LEFT:
                self._steer = -1
        if event.type == pygame.KEYUP:
            if event.key == pygame.K_UP:
                self._throttle = False
            if event.key == pygame.K_DOWN:
                self._brake = False
                self._control.reverse = False
            if event.key == pygame.K_RIGHT:
                self._steer = None
            if event.key == pygame.K_LEFT:
                self._steer = None
    
    # 현재 제어 상태를 처리하고, 키가 계속 눌려있는 경우
    # 제어 매개변수를 변경합니다
    def process_control(self):
        
        if self._throttle: 
            self._control.throttle = min(self._control.throttle + 0.01, 1)
            self._control.gear = 1
            self._control.brake = False
        elif not self._brake:
            self._control.throttle = 0.0
        
        if self._brake:
            # 차가 정지해 있을 때 아래쪽 화살표를 누르고 있으면 후진으로 전환
            if self._vehicle.get_velocity().length() < 0.01 and not self._control.reverse:
                self._control.brake = 0.0
                self._control.gear = 1
                self._control.reverse = True
                self._control.throttle = min(self._control.throttle + 0.1, 1)
            elif self._control.reverse:
                self._control.throttle = min(self._control.throttle + 0.1, 1)
            else:
                self._control.throttle = 0.0
                self._control.brake = min(self._control.brake + 0.3, 1)
        else:
            self._control.brake = 0.0
            
        if self._steer is not None:
            if self._steer == 1:
                self._steer_cache += 0.03
            if self._steer == -1:
                self._steer_cache -= 0.03
            min(0.7, max(-0.7, self._steer_cache))
            self._control.steer = round(self._steer_cache,1)
        else:
            if self._steer_cache > 0.0:
                self._steer_cache *= 0.2
            if self._steer_cache < 0.0:
                self._steer_cache *= 0.2
            if 0.01 > self._steer_cache > -0.01:
                self._steer_cache = 0.0
            self._control.steer = round(self._steer_cache,1)
            
        # 제어 매개변수를 자아 차량에 적용
        self._vehicle.apply_control(self._control)
```

이제 차량과 카메라를 초기화하겠습니다.

```py
# 카메라로 따라갈 차량을 무작위로 선택
ego_vehicle = random.choice(vehicles)

# 차량 뒤에 떠 있는 카메라 초기화
camera_init_trans = carla.Transform(carla.Location(x=-5, z=3), carla.Rotation(pitch=-20))
camera_bp = world.get_blueprint_library().find('sensor.camera.rgb')
camera = world.spawn_actor(camera_bp, camera_init_trans, attach_to=ego_vehicle)

# PyGame 콜백으로 카메라 시작
camera.listen(lambda image: pygame_callback(image, renderObject))

# 카메라 치수 가져오기
image_w = camera_bp.get_attribute("image_size_x").as_int()
image_h = camera_bp.get_attribute("image_size_y").as_int()

# 렌더링 및 차량 제어를 위한 객체 인스턴스화
renderObject = RenderObject(image_w, image_h)
controlObject = ControlObject(ego_vehicle)
```

PyGame 인터페이스를 초기화합니다. 이것은 PyGame을 위한 새 창을 호출합니다.

```py
# 디스플레이 초기화
pygame.init()
gameDisplay = pygame.display.set_mode((image_w,image_h), pygame.HWSURFACE | pygame.DOUBLEBUF)
# 디스플레이를 검은색으로 채우기
gameDisplay.fill((0,0,0))
gameDisplay.blit(renderObject.surface, (0,0))
pygame.display.flip()
```

이제 게임 루프를 시작할 수 있습니다. TM이 제어하는 동안 맵의 다른 차량들을 무작위로 순환하면서 볼 수 있고 교통을 통과하는 여정을 시각화할 수 있습니다. TAB 키를 누르면 무작위로 선택된 새 차량으로 전환되고 RETURN 키를 누르면 키보드의 화살표 키를 통해 차량을 수동으로 제어할 수 있습니다. 이러한 설정은 예를 들어 불규칙한 운전 행동으로 에이전트에게 도전해야 할 경우 유용할 수 있습니다. 선택 로직을 조정하여 에이전트가 운전하는 차량 근처의 차량을 선택할 수 있습니다.

```py
# 게임 루프
crashed = False

while not crashed:
    # 시뮬레이션 시간 진행
    world.tick()
    # 디스플레이 업데이트
    gameDisplay.blit(renderObject.surface, (0,0))
    pygame.display.flip()
    # 현재 제어 상태 처리
    controlObject.process_control()
    # 키 누름 이벤트 수집
    for event in pygame.event.get():
        # 창이 닫히면 while 루프 중단
        if event.type == pygame.QUIT:
            crashed = True
        
        # 제어 상태에 대한 키 누름 이벤트의 효과 구문 분석
        controlObject.parse_control(event)
        if event.type == pygame.KEYUP:
            # TAB 키로 차량 전환
            if event.key == pygame.K_TAB:
                ego_vehicle.set_autopilot(True)
                ego_vehicle = random.choice(vehicles)
                # 차량이 아직 살아있는지 확인(파괴되었을 수 있음)
                if ego_vehicle.is_alive:
                    # 카메라 정지 및 제거
                    camera.stop()
                    camera.destroy()

                    # 새 카메라를 생성하고 새 차량에 부착
                    controlObject = ControlObject(ego_vehicle)
                    camera = world.spawn_actor(camera_bp, camera_init_trans, attach_to=ego_vehicle)
                    camera.listen(lambda image: pygame_callback(image, renderObject))

                    # PyGame 창 업데이트
                    gameDisplay.fill((0,0,0))               
                    gameDisplay.blit(renderObject.surface, (0,0))
                    pygame.display.flip()

# 게임 루프를 종료한 후 카메라를 정지하고 PyGame 종료
camera.stop()
pygame.quit()
```

![수동_제어](../img/tuto_G_pygame/manual_control.gif)