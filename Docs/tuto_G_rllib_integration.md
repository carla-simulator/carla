# RLlib 통합

RLlib 통합은 [Ray/RLlib](https://github.com/ray-project/ray) 라이브러리와 CARLA 사이의 지원을 제공하여 학습 및 추론 목적으로 CARLA 환경을 쉽게 사용할 수 있게 합니다. Ray는 분산 애플리케이션을 구축하기 위한 간단하고 범용적인 API를 제공하는 오픈 소스 프레임워크입니다. Ray는 확장 가능한 강화학습 라이브러리인 RLlib과 확장 가능한 하이퍼파라미터 튜닝 라이브러리인 Tune과 함께 패키징됩니다.

RLlib 통합을 통해 사용자는 CARLA를 Ray의 환경으로 생성하고 사용하여 학습 및 추론 목적으로 활용할 수 있습니다. 이 통합은 로컬과 AWS를 사용한 클라우드 모두에서 바로 사용할 수 있습니다.

이 가이드에서는 RLlib 통합을 로컬과 AWS에서 실행하는 데 필요한 요구사항, 통합 저장소의 구조, 라이브러리 사용 방법에 대한 개요, 그리고 CARLA를 환경으로 사용하는 Ray 실험을 설정하는 예제를 설명합니다.

- [__시작하기 전에__](#시작하기-전에)
    - [로컬 실행을 위한 요구사항](#로컬-실행을-위한-요구사항)
    - [AWS Cloud 실행을 위한 요구사항](#aws-cloud-실행을-위한-요구사항)
- [__RLlib 저장소 구조__](#rllib-저장소-구조)
- [__자체 실험 만들기__](#자체-실험-만들기)
    - [실험 클래스](#1-실험-클래스)
    - [환경 구성](#2-환경-구성)
    - [학습 및 추론 스크립트](#3-학습-및-추론-스크립트)
- [__DQN 예제__](#dqn-예제)
- [__AWS에서 실행__](#aws에서-실행)
    - [AWS 구성](#aws-구성)
    - [학습 AMI 생성](#학습-ami-생성)
    - [클러스터 구성](#클러스터-구성)
    - [학습 실행](#학습-실행)
    - [AWS에서 DQN 예제 실행](#aws에서-dqn-예제-실행)

---

## 시작하기 전에

- [GitHub](https://github.com/carla-simulator/rllib-integration/tree/main)에서 RLlib 통합을 다운로드하거나 저장소를 직접 복제하세요:

```sh
    git clone https://github.com/carla-simulator/rllib-integration.git
```

- 요구사항은 로컬에서 실행하는지 AWS에서 실행하는지에 따라 다릅니다:

>###### 로컬 실행을 위한 요구사항

>>- [CARLA 패키지 버전을 설치](https://github.com/carla-simulator/carla/releases)하고 [추가 에셋](https://carla.readthedocs.io/en/latest/start_quickstart/#import-additional-assets)을 가져오세요. __CARLA 0.9.11이 권장 버전__입니다. 통합이 이 버전으로 설계되고 테스트되었기 때문입니다. 다른 버전도 호환될 수 있지만 완전히 테스트되지 않았으므로 신중하게 사용하세요.
>>- RLlib 통합 저장소의 루트 폴더로 이동하여 Python 요구사항을 설치하세요:

                pip3 install -r requirements.txt

>>- 아래 명령을 실행하거나 `.bashrc` 파일에 `CARLA_ROOT=path/to/carla`를 추가하여 CARLA 패키지의 위치를 지정하는 환경 변수를 설정하세요:

                export CARLA_ROOT=path/to/carla

>###### AWS Cloud 실행을 위한 요구사항

>>- AWS에서 실행하기 위한 요구사항은 RLlib 통합 저장소에 있는 설치 스크립트에서 자동으로 처리됩니다. ["AWS에서 실행"](#aws에서-실행) 섹션에서 자세한 내용을 확인하세요.

---

## RLlib 저장소 구조

저장소는 세 개의 디렉토리로 나뉩니다:

- `rllib_integration`은 CARLA 및 CARLA 서버, 클라이언트, 액터 설정 방법과 관련된 모든 인프라를 포함합니다. 이는 모든 학습 및 테스트 실험이 따라야 하는 기본 구조를 제공합니다.
- `aws`는 AWS 인스턴스에서 실행하는 데 필요한 파일을 포함합니다. `aws_helper.py`는 인스턴스 생성, 데이터 송수신을 포함하여 EC2 인스턴스 관리를 용이하게 하는 여러 기능을 제공합니다.
- `dqn_example`과 루트 디렉토리의 `dqn_*` 파일들은 CARLA를 환경으로 사용하는 Ray 실험을 설정하는 방법에 대한 이해하기 쉬운 예제를 제공합니다.

---

## 자체 실험 만들기

이 섹션에서는 자체 실험을 만드는 방법에 대한 일반적인 개요를 제공합니다. 더 구체적인 예제는 다음 섹션인 ["DQN 예제"](#dqn-예제)를 참조하세요.

최소한 네 개의 파일을 만들어야 합니다:

- 실험 클래스
- 환경 구성
- 학습 및 추론 스크립트

#### 1. 실험 클래스

CARLA 환경을 사용하려면 학습 실험을 정의해야 합니다. Ray는 환경이 특정 정보를 반환하도록 요구합니다. CARLA 환경에 대한 자세한 내용은 [`rllib-integration/rllib_integration/carla_env.py`][carlaEnv]에서 확인할 수 있습니다.

Ray가 요구하는 정보는 특정 실험에 따라 다르므로 모든 실험은 [`BaseExperiment`][baseExperiment]를 상속받아야 합니다. 이 클래스는 자체 실험을 위해 재정의해야 하는 모든 함수를 포함합니다. 이는 모두 학습의 액션, 관측, 보상과 관련된 함수입니다.

[carlaEnv]: https://github.com/carla-simulator/rllib-integration/blob/main/rllib_integration/carla_env.py
[baseExperiment]: https://github.com/carla-simulator/rllib-integration/blob/main/rllib_integration/base_experiment.py#L41

#### 2. 환경 구성

실험은 `.yaml` 파일을 통해 구성해야 합니다. 구성 파일을 통해 전달된 설정은 기본 설정을 재정의합니다. 다양한 기본 설정의 위치는 아래에서 설명합니다.

구성 파일에는 세 가지 주요 용도가 있습니다:

1. 타임아웃이나 맵 품질과 같은 대부분의 CARLA 서버 및 클라이언트 설정을 설정합니다. 기본값은 [여기][defaultCarlaSettings]에서 확인할 수 있습니다.
2. 실험에 특화된 변수를 설정하고 도시 조건, 자아 차량 및 센서의 스폰을 지정합니다. 기본 설정은 [여기][defaultExperimentSettings]에서 찾을 수 있으며 센서 설정 방법의 예시를 제공합니다.
3. [Ray의 학습][raySettings]에 특화된 설정을 구성합니다. 이러한 설정은 사용되는 특정 트레이너와 관련이 있습니다. 내장 모델을 사용하는 경우 여기에서 해당 설정을 적용할 수 있습니다.

[defaultCarlaSettings]: https://github.com/carla-simulator/rllib-integration/blob/main/rllib_integration/carla_core.py#L23
[defaultExperimentSettings]: https://github.com/carla-simulator/rllib-integration/blob/main/rllib_integration/base_experiment.py#L12
[raySettings]: https://github.com/ray-project/ray/blob/master/rllib/agents/trainer.py

#### 3. 학습 및 추론 스크립트

마지막 단계는 자체 학습 및 추론 스크립트를 만드는 것입니다. 이 부분은 전적으로 여러분에게 달려 있으며 Ray API에 따라 달라집니다. 자체 특정 모델을 만들고 싶다면 [Ray의 커스텀 모델 문서][rayCustomModel]를 확인하세요.

[rayCustomModel]: https://docs.ray.io/en/master/rllib-models.html#custom-models-implementing-your-own-forward-logic

---

## DQN 예제

이 섹션은 이전 섹션을 기반으로 [BirdView 의사센서][birdview]와 Ray의 [DQNTrainer][dqntrainer]를 사용하여 RLlib 통합으로 작업하는 구체적인 예제를 보여줍니다.

[birdview]: https://github.com/carla-simulator/rllib-integration/blob/main/rllib_integration/sensors/bird_view_manager.py
[dqntrainer]: https://github.com/ray-project/ray/blob/master/rllib/agents/dqn/dqn.py#L285

DQN 예제의 구조는 다음과 같습니다:

- __실험 클래스__: `BaseExperiment` 클래스의 메서드를 재정의하는 [`DQNExperiment`][dqnExperiment]
- __구성 파일__: [`dqn_example/dqn_config.yaml`][dqnConfig]
- __학습 파일__: [`dqn_train.py`][dqnTrain]
- __추론 파일__:
    - __Ray 사용__: [`dqn_inference_ray.py`][dqnInferenceRay]
    - __Ray 미사용__: [`dqn_inference.py`][dqnInference]

[dqnExperiment]: https://github.com/carla-simulator/rllib-integration/blob/main/dqn_example/dqn_experiment.py#L19
[dqnConfig]: https://github.com/carla-simulator/rllib-integration/blob/main/dqn_example/dqn_config.yaml
[dqnTrain]: https://github.com/carla-simulator/rllib-integration/blob/main/dqn_train.py
[dqnInferenceRay]: https://github.com/carla-simulator/rllib-integration/blob/main/dqn_inference_ray.py
[dqnInference]: https://github.com/carla-simulator/rllib-integration/blob/main/dqn_inference.py

로컬에서 예제를 실행하려면:

1. pytorch 설치:

        pip3 install -r dqn_example/dqn_requirements.txt

2. 학습 파일 실행:

        python3 dqn_train.py dqn_example/dqn_config.yaml --name dqn        

!!! 참고
    기본 구성은 1개의 GPU와 12개의 CPU를 사용합니다. 로컬 머신에 그만큼의 용량이 없다면 [구성 파일][dqnConfig]에서 숫자를 낮추세요.
    
    메모리 부족 문제가 발생하면 `buffer_size` 매개변수를 줄이는 것을 고려하세요.

---

## AWS에서 실행

이 섹션에서는 RLlib 통합을 사용하여 AWS EC2 인스턴스에서 자동으로 학습과 추론을 실행하는 방법을 설명합니다. 인스턴스의 스케일링을 처리하기 위해 [Ray autoscaler API][rayAutoscaler]를 사용합니다.

[rayAutoscaler]: https://docs.ray.io/en/latest/cluster/index.html

#### AWS 구성

boto3 환경을 올바르게 구성해야 합니다. 자세한 내용은 [여기][awsBoto3]를 확인하세요.

[awsBoto3]: https://boto3.amazonaws.com/v1/documentation/api/latest/guide/configuration.html

#### 학습 AMI 생성

제공된 [`aws_helper.py`][awsHelper] 스크립트를 사용하여 베이스 이미지의 이름과 [`rllib-integration/aws/install`][installsh]에 있는 설치 스크립트 `install.sh`를 전달하여 아래 명령으로 학습에 필요한 이미지를 자동으로 생성하세요:

        python3 aws_helper.py create-image --name <AMI-name> --installation-scripts <installation-scripts> --instance-type <instance-type> --volume-size <volume-size>

[awsHelper]: https://github.com/carla-simulator/rllib-integration/blob/main/aws/aws_helper.py
[installsh]: https://github.com/carla-simulator/rllib-integration/blob/main/aws/install/install.sh

#### 클러스터 구성

이미지가 생성되면 이미지 정보가 포함된 출력이 나타납니다. Ray autoscaler를 사용하려면 [autoscaler 구성 파일][autoscalerSettings]의 `<ImageId>`와 `<SecurityGroupIds>` 설정을 출력의 정보로 업데이트하세요.

[autoscalerSettings]: https://docs.ray.io/en/latest/cluster/config.html

#### 학습 실행

이미지가 생성되면 Ray의 API를 사용하여 클러스터에서 학습을 실행할 수 있습니다:

1. 클러스터 초기화:

        ray up <autoscaler_configuration_file>

2. (선택사항) 클러스터 초기화 후 로컬 코드가 수정된 경우 이 명령을 실행하여 업데이트하세요:

        ray rsync-up <autoscaler_configuration_file> <path_to_local_folder> <path_to_remote_folder>

3. 학습 실행:

        ray submit <autoscaler_configuration_file> <training_file>

4. (선택사항) 클러스터 상태 모니터링:

        ray attach <autoscaler_configuration_file>
        watch -n 1 ray status

5. 클러스터 종료:

        ray down <autoscaler_configuration_file>

#### AWS에서 DQN 예제 실행

AWS에서 DQN 예제를 실행하려면:

1. [`dqn_example/dqn_autoscaler.yaml`][dqnAutoscaler] 구성을 다음 명령에 전달하여 이미지를 생성하세요:

        python3 aws_helper.py create-image --name <AMI-name> --installation-scripts install/install.sh --instance-type <instance-type> --volume-size <volume-size>

[dqnAutoscaler]: https://github.com/carla-simulator/rllib-integration/blob/main/dqn_example/dqn_autoscaler.yaml

2. 이전 명령에서 제공한 정보로 [`dqn_autoscaler.yaml`][dqnAutoscaler]의 `<ImageId>`와 `<SecurityGroupIds>` 설정을 업데이트하세요.

3. 클러스터 초기화:

        ray up dqn_example/dqn_autoscaler.yaml

4. (선택사항) 로컬 변경사항으로 원격 파일 업데이트:

        ray rsync-up dqn_example/dqn_autoscaler.yaml dqn_example .
        ray rsync-up dqn_example/dqn_autoscaler.yaml rllib_integration .

5. 학습 실행:

        ray submit dqn_example/dqn_autoscaler.yaml dqn_train.py -- dqn_example/dqn_config.yaml --auto

6. (선택사항) 클러스터 상태 모니터링:

        ray attach dqn_example/dqn_autoscaler.yaml
        watch -n 1 ray status

7. 클러스터 종료:

        ray down dqn_example/dqn_autoscaler.yaml

---

이 가이드에서는 RLlib 통합을 AWS와 로컬 머신에 설치하고 실행하는 방법을 설명했습니다. 가이드를 따라하면서 질문이나 문제가 있다면 [포럼](https://github.com/carla-simulator/carla/discussions/)에 게시하거나 [GitHub](https://github.com/carla-simulator/rllib-integration)에 이슈를 제기하세요.