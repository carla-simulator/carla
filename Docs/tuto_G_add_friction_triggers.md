# 마찰 트리거 추가 방법

*마찰 트리거*는 런타임에 추가할 수 있는 박스 트리거로, 사용자가 이러한 유형의 트리거 내부에 있을 때 차량 바퀴의 마찰력을 다르게 정의할 수 있게 합니다. 예를 들어, 이는 맵의 특정 영역을 동적으로 미끄러운 표면으로 만드는 데 유용할 수 있습니다.

PythonAPI를 사용하여 마찰 트리거를 생성하려면, 사용자는 먼저 `static.trigger.friction` 블루프린트 정의를 가져온 다음, 해당 블루프린트 정의에 다음과 같은 필수 속성들을 설정해야 합니다:

- *friction*: 차량이 내부에 있을 때 트리거 박스의 마찰력.
- *extent_x*: 경계 상자의 X 좌표 범위(센티미터 단위).
- *extent_y*: 경계 상자의 Y 좌표 범위(센티미터 단위).
- *extent_z*: 경계 상자의 Z 좌표 범위(센티미터 단위).

이를 완료한 후, 마찰 트리거의 위치와 회전을 지정하기 위한 변환(transform)을 정의하고 생성합니다.

##### 예제

```py
import carla

def main():
    # 클라이언트 연결
    client = carla.Client('127.0.0.1', 2000)
    client.set_timeout(2.0)

    # 월드와 액터 가져오기
    world = client.get_world()
    actors = world.get_actors()

    # 트리거 마찰 블루프린트 찾기
    friction_bp = world.get_blueprint_library().find('static.trigger.friction')

    extent = carla.Location(700.0, 700.0, 700.0)

    friction_bp.set_attribute('friction', str(0.0))
    friction_bp.set_attribute('extent_x', str(extent.x))
    friction_bp.set_attribute('extent_y', str(extent.y))
    friction_bp.set_attribute('extent_z', str(extent.z))

    # 트리거 마찰 생성
    transform = carla.Transform()
    transform.location = carla.Location(100.0, 0.0, 0.0)
    world.spawn_actor(friction_bp, transform)

    # 트리거 시각화를 위한 선택적 코드
    world.debug.draw_box(box=carla.BoundingBox(transform.location, extent * 1e-2), rotation=transform.rotation, life_time=100, thickness=0.5, color=carla.Color(r=255,g=0,b=0))

if __name__ == '__main__':
    main()
```