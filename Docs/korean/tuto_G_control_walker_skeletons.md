# 보행자 뼈대 제어

이 튜토리얼에서는 CARLA Python API를 사용하여 보행자의 골격을 수동으로 제어하고 애니메이션을 적용하는 방법을 설명합니다. 사용 가능한 모든 클래스와 메서드의 참조는 [Python API 참조](python_api.md)에서 찾을 수 있습니다.

* [__보행자 골격 구조__](#보행자-골격-구조)  
* [__보행자 뼈대 수동 제어__](#보행자-뼈대-수동-제어)  
    * [시뮬레이터 연결](#시뮬레이터-연결)  
    * [보행자 생성](#보행자-생성)  
    * [보행자 골격 제어](#보행자-골격-제어)  

!!! 참고
    **이 문서는 사용자가 Python API에 익숙하다고 가정합니다**. <br>
    이 문서를 읽기 전에 사용자는 첫 단계 튜토리얼을 읽어야 합니다.
    [핵심 개념](core_concepts.md).

---
## 보행자 골격 구조

모든 보행자는 동일한 골격 계층 구조와 뼈대 이름을 가지고 있습니다. 아래는 골격 계층 구조의 이미지입니다.

```
crl_root
└── crl_hips__C
    ├── crl_spine__C
    │   └── crl_spine01__C
    │       ├── ctrl_shoulder__L
    │       │   └── crl_arm__L
    │       │       └── crl_foreArm__L
    │       │           └── crl_hand__L
    │       │               ├── crl_handThumb__L
    │       │               │   └── crl_handThumb01__L
    │       │               │       └── crl_handThumb02__L
    │       │               │           └── crl_handThumbEnd__L
    │       │               ├── crl_handIndex__L
    │       │               │   └── crl_handIndex01__L
    │       │               │       └── crl_handIndex02__L
    │       │               │           └── crl_handIndexEnd__L
    │       │               ├── crl_handMiddle_L
    │       │               │   └── crl_handMiddle01__L
    │       │               │       └── crl_handMiddle02__L
    │       │               │           └── crl_handMiddleEnd__L
    │       │               ├── crl_handRing_L
    │       │               │   └── crl_handRing01__L
    │       │               │       └── crl_handRing02__L
    │       │               │           └── crl_handRingEnd__L
    │       │               └── crl_handPinky_L
    │       │                   └── crl_handPinky01__L
    │       │                       └── crl_handPinky02__L
    │       │                           └── crl_handPinkyEnd__L
    │       ├── crl_neck__C
    │       │   └── crl_Head__C
    │       │       ├── crl_eye__L
    │       │       └── crl_eye__R
    │       └── crl_shoulder__R
    │           └── crl_arm__R
    │               └── crl_foreArm__R
    │                   └── crl_hand__R
    │                       ├── crl_handThumb__R
    │                       │   └── crl_handThumb01__R
    │                       │       └── crl_handThumb02__R
    │                       │           └── crl_handThumbEnd__R
    │                       ├── crl_handIndex__R
    │                       │   └── crl_handIndex01__R
    │                       │       └── crl_handIndex02__R
    │                       │           └── crl_handIndexEnd__R
    │                       ├── crl_handMiddle_R
    │                       │   └── crl_handMiddle01__R
    │                       │       └── crl_handMiddle02__R
    │                       │           └── crl_handMiddleEnd__R
    │                       ├── crl_handRing_R
    │                       │   └── crl_handRing01__R
    │                       │       └── crl_handRing02__R
    │                       │           └── crl_handRingEnd__R
    │                       └── crl_handPinky_R
    │                           └── crl_handPinky01__R
    │                               └── crl_handPinky02__R
    │                                   └── crl_handPinkyEnd__R
    ├── crl_thigh__L
    │   └── crl_leg__L
    │       └── crl_foot__L
    │           └── crl_toe__L
    │               └── crl_toeEnd__L
    └── crl_thigh__R
        └── crl_leg__R
            └── crl_foot__R
                └── crl_toe__R
                    └── crl_toeEnd__R
```

---
## 보행자 뼈대 수동 제어

다음은 CARLA Python API에서 보행자의 뼈대 변환을 변경하는 방법을 단계별로 설명한 자세한 예제입니다.

### 시뮬레이터 연결

이 예제에서 사용되는 필요한 라이브러리를 임포트합니다.

```py
import carla
import random
```

carla 클라이언트를 초기화합니다.

```py
client = carla.Client('127.0.0.1', 2000)
client.set_timeout(2.0)
```

### 보행자 생성

맵의 생성 지점 중 하나에서 무작위 보행자를 생성합니다.

```py
world = client.get_world()
blueprint = random.choice(self.world.get_blueprint_library().filter('walker.*'))
spawn_points = world.get_map().get_spawn_points()
spawn_point = random.choice(spawn_points) if spawn_points else carla.Transform()
world.try_spawn_actor(blueprint, spawn_point)
```

### 보행자 골격 제어

보행자의 골격은 WalkerBoneControl 클래스의 인스턴스를 보행자의 apply_control 함수에 전달하여 수정할 수 있습니다. WalkerBoneControl 클래스는 수정할 뼈대의 변환을 포함합니다. 이 클래스의 bone_transforms 멤버는 값 쌍의 튜플 리스트로, 첫 번째 값은 뼈대 이름이고 두 번째 값은 뼈대 변환입니다. apply_control 함수는 매 틱마다 호출하여 보행자의 골격에 애니메이션을 적용할 수 있습니다. 각 변환의 위치와 회전은 부모를 기준으로 합니다. 따라서 부모 뼈대의 변환이 수정되면 모델 공간에서 자식 뼈대의 변환도 상대적으로 변경됩니다.

아래 예제에서는 보행자의 손 회전을 전방 축을 기준으로 90도로 설정하고 위치를 원점으로 설정합니다.

```py
control = carla.WalkerBoneControl()
first_tuple = ('crl_hand__R', carla.Transform(rotation=carla.Rotation(roll=90)))
second_tuple = ('crl_hand__L', carla.Transform(rotation=carla.Rotation(roll=90)))
control.bone_transforms = [first_tuple, second_tuple]
world.player.apply_control(control)
```