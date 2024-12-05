# Town 12

![town_12](../img/catalogue/maps/town12/town12.webp)

Town 12는 10x10 km<sup>2</sup> 크기의 대형 맵입니다. 대부분 2x2 km<sup>2</sup> 크기(일부 가장자리 타일은 더 작음)의 36개 타일로 나뉘어 있습니다. 도로 배치는 [미국 텍사스 주의 아마릴로 시](https://www.google.com/maps/place/Amarillo,+TX,+USA/@35.2018863,-101.9450251,11z/data=!3m1!4b1!4m5!3m4!1s0x870148d4b245cf03:0xd0f3d11c6836d2af!8m2!3d35.2219971!4d-101.8312969)의 도로 배치에서 부분적으로 영감을 받았습니다. 도시에는 도심, 주거 및 시골 지역을 포함한 여러 대조적인 지역이 있으며, 순환도로가 있는 대규모 고속도로 시스템이 도시를 둘러싸고 있습니다. 건축 스타일은 북미 전역의 중소도시들의 특징을 반영합니다.

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
![town12_항공사진](../img/catalogue/maps/town12/town12roadrunner.webp#map)

__CARLA 좌표__:

* __X__:  <span id="carlacoord_x" style="animation: fadeMe 2s;">--</span>
* __Y__:  <span id="carlacoord_y" style="animation: fadeMe 2s;">--</span>

관심 지점을 더블 클릭하면 네비게이터가 해당하는 CARLA 좌표를 표시하고 아래 코드 블록에서 업데이트합니다. 코드를 노트북이나 Python 터미널에 복사하여 원하는 위치로 관찰자를 이동할 수 있습니다. 먼저 [클라이언트를 연결하고 월드 객체를 설정](tuto_first_steps.md#launching-carla-and-connecting-the-client)해야 합니다:

```py
# CARLA 좌표: X 0.0, Y 0.0
spectator = world.get_spectator()
loc = carla.Location(0.0, 0.0, 500.0)
rot = carla.Rotation(pitch=-90, yaw=0.0, roll=0.0)
spectator.set_transform(carla.Transform(loc, rot))
```

## Town 12 구역

#### 고층 도심:

Town 12의 도심 지역은 많은 미국과 유럽 대도시의 도심 지역과 비슷하게, 일관된 도로 격자 위에 배치된 고층 빌딩들이 넓게 펼쳐져 있습니다.

![고층건물](../img/catalogue/maps/town12/high_rise.webp)

#### 고밀도 주거지역:

Town 12의 고밀도 주거지역에는 1층에 카페와 소매점과 같은 상업시설이 있는 2-10층 높이의 아파트 건물이 많이 있습니다.

![고밀도_주거](../img/catalogue/maps/town12/hi_dens_res.webp)
#### 커뮤니티 건물:

도시의 도심 지역 옆에 위치한 커뮤니티 건물들은 1층에 카페와 부티크가 있는 다채로운 보헤미안 스타일의 2-4층 아파트 건물들로 구성되어 있습니다.

![커뮤니티](../img/catalogue/maps/town12/community.webp)

#### 저밀도 주거지역:

Town 12의 저밀도 주거지역은 많은 미국 도시의 전형적인 교외 지역을 반영하며, 울타리가 있는 정원과 차고가 있는 1층과 2층 주택들로 이루어져 있습니다.

![저밀도_주거](../img/catalogue/maps/town12/low_dens_res.webp)

#### 공원:

밀집된 주거지역과 도심 지역 사이사이에 작은 녹지 공용 공간이 있어, 도시 건축물과 녹색 수목이 대조를 이루고 있습니다.

![공원](../img/catalogue/maps/town12/parks.webp)

#### 고속도로와 교차로:

Town 12는 3-4차선 고속도로와 인상적인 로터리 교차로를 포함하는 광범위한 고속도로 시스템을 갖추고 있습니다.

![고속도로](../img/catalogue/maps/town12/highway.webp)

#### 시골과 농지:

Town 12에는 나무로 된 헛간과 농가, 풍차, 곡물 저장고, 옥수수 밭, 건초 더미, 시골 울타리 같은 전형적인 농지 건물들이 있는 시골 지역도 있습니다. 이 지역에는 표시가 없는 시골 비포장도로와 도시 간 교통을 위한 단일 차선 도시 간 도로가 있습니다.

![시골](../img/catalogue/maps/town12/rural.webp)

#### 수역:

Town 12에는 2개의 큰 호수와 여러 연못을 포함한 여러 수역이 있습니다. 도시 근처에 위치한 큰 수역들은 스카이라인의 반전된 반사를 만들어내어 자율 주행 에이전트에게 도전적인 환경을 제공합니다.

![수역](../img/catalogue/maps/town12/water.webp)