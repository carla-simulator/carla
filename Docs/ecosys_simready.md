# SimReady 콘텐츠와 NVIDIA Omniverse

NVIDIA의 [__SimReady__](https://developer.nvidia.com/omniverse/simready-assets) 사양은 로봇공학과 자율주행에서 머신러닝 목적으로 3D 가상 환경을 시뮬레이션하기 위한 __시뮬레이션 전용 3D 콘텐츠__ 제작을 지원하여 콘텐츠 제작 파이프라인을 간소화합니다. CARLA에 통합된 Omniverse Unreal Engine 플러그인을 통해, 단 몇 번의 클릭만으로 조명, 도어, 휠이 이미 설정된 차량 및 CARLA 맵을 즉시 꾸밀 수 있는 소품 등 SimReady 콘텐츠를 손쉽게 가져올 수 있습니다. CARLA의 Omniverse 통합은 환경 제작 파이프라인을 크게 가속화하며 [__Omniverse 생태계__](https://www.nvidia.com/en-us/omniverse/ecosystem/)의 다양한 응용 프로그램에 접근할 수 있는 기회를 제공합니다.

!!! note  
    Omniverse Unreal Engine Connector는 현재 Windows에서만 사용할 수 있습니다.

다음은 CARLA에서 Omniverse와 SimReady 콘텐츠를 사용하는 방법입니다.  

## 시작하기 전  
가장 먼저 [NVIDIA Omniverse를 설치](https://docs.omniverse.nvidia.com/install-guide/latest/index.html)해야 합니다.  

---

## 1. Unreal Engine Omniverse Connector 설치  

1. NVIDIA Omniverse Launcher를 실행합니다.  
2. **Exchange** 탭으로 이동합니다.  
3. __Epic Games Unreal Engine 4.26 Omniverse Connector__를 찾습니다.  
4. 설치 버전이 **Release 105.1.578**인지 확인합니다.  
5. *Install*을 클릭합니다.  
6. Unreal Engine을 찾을 수 없다는 메시지가 표시되면 **OK**를 클릭합니다.  
7. Unreal Engine 설치 경로를 선택하라는 메시지가 표시되면 다음 경로를 선택합니다: `{UE4_ROOT}\Engine\Plugins\Marketplace\NVIDIA`  
8. *Install*을 클릭합니다.  

---

## 2. 로컬 Omniverse 서버 설정  

1. NVIDIA Omniverse Launcher에서 Omniverse를 엽니다.  
2. *Nucleus* 탭으로 이동합니다.  
3. *Create Local Server*를 클릭합니다.  
4. 관리자 정보를 입력하여 생성합니다.  
5. *Local Nucleus Service* 옆의 폴더 아이콘을 클릭하면 웹 브라우저에서 로컬 서버가 열립니다.  

---

## 3. CARLA 시뮬레이터와 연결  

1. CARLA 루트 폴더에서 명령어 `make launch`를 실행하여 CARLA를 실행합니다.  
2. 기존 서버를 초기화하려면 *Clean Local Assets*를 클릭합니다 (선택 사항).  
3. *Omniverse* 아이콘을 선택하고 *Add Server*를 클릭합니다.  
4. 서버 이름을 지정하고 *Add to Content Browser*를 클릭합니다.  
5. 브라우저에서 로그인 폼이 열리면 *Create Account*를 클릭합니다.  
6. 관리자 정보를 입력하여 계정을 생성합니다.  
7. Unreal Engine 콘텐츠 브라우저의 `Content/Omniverse` 폴더에서 Omniverse 폴더를 확인합니다.  
8. 브라우저에서 *Connect to a Server*를 클릭합니다.  
9. CARLA 서버 이름으로 인증합니다.  
10. 설정한 관리자 정보를 사용하여 로그인합니다.  
11. 서버 폴더가 브라우저에 표시됩니다.  

---

## 4. SimReady 에셋 가져오기  

1. 브라우저 탐색기의 Projects 폴더로 이동합니다.  
2. 마우스 오른쪽 버튼을 클릭하고 *Upload Folder*를 선택합니다.  
3. SimReady 폴더를 선택합니다.  
4. 파일을 업로드합니다.  

---

## 5. Omniverse Connector를 사용하여 차량을 CARLA에 로드  

1. Unreal Engine에서 CARLA 프로젝트를 엽니다.  
2. `CarlaTools/Content/USDImporter`로 이동합니다.  
3. *UW_USDVehicleImporterEditorWidget*에서 마우스 오른쪽 버튼을 클릭합니다.  
4. *Run Editor Utility Widget*을 선택합니다.  
5. Omniverse 브라우저 탐색기에서 차량을 찾습니다.  
6. 경로를 복사합니다 (예: `omniverse://localhost/Projects/SimReadyUSD.../vehicle.usd`).  
7. 위젯의 *Omniverse URL* 탭에 경로를 붙여넣습니다.  
8. *Import Asset*을 선택합니다.  
9. 지정한 *Import destination* 폴더에 차량이 나타납니다.  
10. 다른 맵 씬을 열고, *Save Content* 프롬프트에서 *Untitled* 씬의 선택을 해제하고 *Save selected*를 클릭합니다.  
11. 차량은 이제 CARLA에서 사용 가능하며 Python API를 통해 접근할 수 있습니다.  