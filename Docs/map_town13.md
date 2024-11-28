# Town 13

![town_13](../img/catalogue/maps/town13/town13montage.webp)

Town 13은 10x10 km<sup>2</sup> 크기의 대형 맵입니다. 대부분 2x2 km<sup>2</sup> 크기(일부 가장자리 타일은 더 작음)의 36개 타일로 나뉘어 있습니다. 도시에는 도심, 주거 및 시골 지역을 포함한 여러 대조적인 지역이 있으며, 순환도로가 있는 대규모 고속도로 시스템이 도시를 둘러싸고 있습니다. 건축 스타일은 북미 전역의 중소도시들의 특징을 반영합니다.

!!! 참고
    Town 13은 Town 12의 보조 도시로 설계되어 **학습-테스트 쌍**으로 사용될 수 있습니다. 두 도시는 많은 공통적인 특징을 공유하지만, 건물 스타일, 도로 텍스처, 보도 텍스처, 식생 등에서 차이점도 많이 있습니다. 한 도시에서 학습 데이터를 생성하고 다른 도시에서 테스트하는 것은 자율주행 스택 개발 중에 발생할 수 있는 과적합 문제를 파악하는 데 이상적입니다.

## 네비게이터

네비게이터 인터랙티브 맵을 사용하여 마을을 둘러보고 CARLA 시뮬레이터에서 사용할 좌표를 도출할 수 있습니다.

__네비게이터 사용법__:

* `마우스 왼쪽 버튼` - 클릭하고 누른 채로 좌, 우, 위, 아래로 드래그하여 맵을 이동
* `마우스 휠 스크롤` - 아래로 스크롤하여 축소, 위로 스크롤하여 마우스 포인터 위치를 확대
* `더블 클릭` - 맵의 한 지점을 더블 클릭하여 좌표를 기록, 맵 아래의 텍스트와 코드 블록에서 좌표를 확인할 수 있음

__구역 색상 참조__:

* <span style="color:#595d5e; background-color:#595d5e;">&nbsp</span>   [초고층 건물](#high-rise-downtown)
* <span style="color:#d2dddc; background-color:#d2dddc;">&nbsp</span>   [고밀도 주거지역](#high-density-residential)
* <span style="color:#838c8b; background-color:#838c8b;">&nbsp</span>   [커뮤니티 건물](#community-buildings)
* <span style="color:#17d894; background-color:#17d894;">&nbsp</span>   [저밀도 주거지역](#low-density-residential)
* <span style="color:#df6a19; background-color:#df6a19;">&nbsp</span>   [공원](#parks)
* <span style="color:#839317; background-color:#839317;">&nbsp</span>   [시골 농지](#rural-and-farmland)
* <span style="color:#265568; background-color:#265568;">&nbsp</span>   [수역](#water)
![town13_항공사진](../img/catalogue/maps/town13/town13roadrunner.webp#map)

__CARLA 좌표__:

* __X__: <span id="carlacoord_x" style="animation: fadeMe 2s;">--</span>
* __Y__: <span id="carlacoord_y" style="animation: fadeMe 2s;">--</span>

관심 지점을 더블 클릭하면 네비게이터가 해당하는 CARLA 좌표를 표시하고 아래 코드 블록에서 업데이트합니다. 코드를 노트북이나 Python 터미널에 복사하여 원하는 위치로 관찰자를 이동할 수 있습니다. 먼저 [클라이언트를 연결하고 월드 객체를 설정](tuto_first_steps.md#launching-carla-and-connecting-the-client)해야 합니다:

```py
# CARLA 좌표: X 0.0, Y 0.0
spectator = world.get_spectator()
loc = carla.Location(0.0, 0.0, 300.0)
rot = carla.Rotation(pitch=-90, yaw=0.0, roll=0.0)
spectator.set_transform(carla.Transform(loc, rot))
```

## Town 13 구역

#### 고층 도심:

Town 13의 도심 지역은 많은 미국과 유럽 대도시의 도심 지역과 비슷하게, 일관된 도로 격자 위에 배치된 고층 빌딩들이 넓게 펼쳐져 있습니다.

![고층건물](../img/catalogue/maps/town13/high_rise.webp)

#### 커뮤니티 건물:

커뮤니티 건물들은 도시의 도심 지역 옆에 위치한 다채로운 보헤미안 스타일의 2-4층 아파트 건물들로, 1층에는 상업 시설이 있습니다.

![커뮤니티](../img/catalogue/maps/town13/community.webp)
#### 고밀도 주거지역:

Town 13의 고밀도 주거지역에는 1층에 카페와 소매점과 같은 상업시설이 있는 2-10층 높이의 아파트 건물이 많이 있습니다. 많은 주거용 블록에는 남부 유럽 국가들과 비슷한 햇빛 가리개가 있는 발코니가 있습니다.

![고밀도_주거](../img/catalogue/maps/town13/high_dens_res.webp)

#### 저밀도 주거지역:

Town 13의 저밀도 주거지역은 많은 유럽 도시의 교외 지역을 반영하며, 울타리가 있는 정원과 차고가 있는 1층과 2층 주택들로 이루어져 있습니다.

![저밀도_주거](../img/catalogue/maps/town13/low_dens_res.webp)

#### 공원:

밀집된 주거지역과 도심 지역 사이사이에 작은 녹지 공용 공간이 있어, 도시 건축물과 녹색 수목이 대조를 이루고 있습니다.

![공원](../img/catalogue/maps/town13/parks.webp)

#### 고속도로와 교차로:

Town 13은 3-4차선 고속도로, 대형 로터리, 그리고 큰 수역을 가로지르는 제방도로를 포함하는 광범위한 고속도로 시스템을 갖추고 있습니다.

![고속도로](../img/catalogue/maps/town13/highway.webp)

#### 시골과 농지:

Town 13에는 나무로 된 헛간과 농가, 풍차, 곡물 저장고, 옥수수 밭, 건초 더미, 시골 울타리 같은 전형적인 농지 건물들이 있는 시골 지역도 있습니다. 이 지역에는 표시가 없는 시골 비포장도로와 도시 간 교통을 위한 단일 차선 도시 간 도로가 있습니다.

![시골](../img/catalogue/maps/town13/rural.webp)

#### 수역:

Town 13에는 중앙에 섬이 있는 큰 호수와 시골 지역의 여러 연못을 포함한 다수의 수역이 있습니다.

![수역](../img/catalogue/maps/town13/water.webp)