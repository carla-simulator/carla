# Town 12

Town 12 is a Large Map with dimensions of 10x10 km<sup>2</sup>. It is divided into 25 tiles, each with dimensions of 2x2 km<sup>2</sup>. The road layout is partially inspired by the road layout of the city of Amarillo in Texas, USA. There are numerous regions to the city including urban, residential and rural areas, along with a large highway system surrounding the city with a ringroad. The architectural styles reflect those of many medium to large cities across North America.  

## Navigator

The navigator interactive map can be used to browse the town and derive coordinates to use in the CARLA simulator.

__Using the navigator__:

* `left mouse button` - click and hold, drag downwards to zoom in, drag upwards to zoom out
* `middle mouse button` or `press mouse wheel` - click and hold, drag left, right, up or down to move the map
* `double click` - double click on a point on the map to record the coordinates, you will find the coordinates in the code block just below the map

__Zone color reference__:

* <span style="color:#595d5e; background-color:#595d5e;">&nbsp</span>   [Skyscraper](#high-rise-downtown)
* <span style="color:#d2dddc; background-color:#d2dddc;">&nbsp</span>   [High density residential](#high-density-residential)
* <span style="color:#838c8b; background-color:#838c8b;">&nbsp</span>   [Community buildings](#community-buildings)
* <span style="color:#17d894; background-color:#17d894;">&nbsp</span>   [Low density residential](#low-density-residential)
* <span style="color:#df6a19; background-color:#df6a19;">&nbsp</span>   [Parks](#parks)
* <span style="color:#839317; background-color:#839317;">&nbsp</span>   [Rural farmland](#rural-and-farmland)
* <span style="color:#265568; background-color:#265568;">&nbsp</span>   [Water](#water)



![town12_aerial](../img/maps/town12/town12roadrunner.webp#map)

__CARLA coordinates__:

Copy and paste the following code into a notebook or Python terminal to translate the spectator to the desired location. You will need first to [connect the client and set up the world object](tuto_first_steps.md#launching-carla-and-connecting-the-client):

```py
spectator = world.get_spectator()
loc = carla.Location(0.0, 0.0, 500.0)
rot = carla.Rotation(pitch=-90, yaw=0.0, roll=0.0)
spectator.set_transform(carla.Transform(loc, rot))
```
## Town 12 zones

#### High-rise downtown:

Town 12's downtown area is a large span of high rise skyscrapers arranged into blocks on a consistent grid of roads, resembling downtown areas in many large American and European cities.

![high_rise](../img/maps/town12/high_rise.webp)

#### High density residential:

The high density residential areas of Town 12 have many 2-10 storey apartment buildings with commercial properties like cafes and retail stores at street level.

![high_dens_res](../img/maps/town12/hi_dens_res.webp)


#### Community buildings:

The community buildings have many 2-3 storey apartment buildings in a bohemian style with cafes and boutiques on the ground floors.

![community](../img/maps/town12/community.webp)

#### Low density residential:

The low density residential regions of Town 12 reflect the classic suburbs of many American cities, with one and two story homes surrounded by fenced gardens and garages.

![low_dens_res](../img/maps/town12/low_dens_res.webp)

#### Parks:

The dense residential and downtown areas are broken up by small islands of green communal space, juxtaposing foliage against urban architecture.

![parks](../img/maps/town12/parks.webp)

#### Highways and intersections:

Town 12 has an extensive highway system, including 3-4 lane highways interspersed with impressive roundabout junctions and intersections.

![highway](../img/maps/town12/highway.webp)

#### Rural and farmland:

Town 12 also has rural regions with characteristic farmland buildings like wooden barns and farmhouses, windmills, grain silos, corn fields, hay bails and rural fencing. 

![rural](../img/maps/town12/rural.webp)

#### Water:

There are several bodies of water in town 12 including 2 large lakes and several ponds. 

![water](../img/maps/town12/water.webp)

<script>
window.addEventListener('load', function () {

    const coords = document.getElementsByClassName("hljs-number")
  
    const image = document.querySelector('[src$="map"]');
    const canv = document.createElement('canvas');
    canv.setAttribute('height', image.height)
    canv.setAttribute('width', image.width)
    image.parentNode.replaceChild(canv, image)

    var state = {mDown: false, button: 0, lastX: 0, lastY:0, canvX: 0, canvY: 0, zoom: 1.0, mdownX: 0, mdownY: 0, pX: 0.5, pY: 0.5, dblClick: false, listObj: false}

    ctx = canv.getContext('2d')
    ctx.drawImage(image, 0, 0, canv.width, canv.height)

    canv.addEventListener('mousemove', (event) => {
        dX = event.clientX - state.lastX
        dY = event.clientY - state.lastY
        state.lastX = event.clientX
        state.lastY = event.clientY

        if(state.mDown && state.button == 1) {
            state.canvX += dX
            state.canvY += dY
            ctx.clearRect(0, 0, canv.width, canv.height)
            ctx.drawImage(image,  state.canvX, state.canvY, canv.width * state.zoom, canv.height * state.zoom)
        }
        if(state.mDown && state.button == 0) {
            if(dY > 0){
                state.zoom *= 1.05 
            } else {
               state.zoom *= 0.95
            }
            ctx.clearRect(0, 0, canv.width, canv.height)

            state.canvX = - canv.width * state.zoom * state.pX + state.mdownX;
            state.canvY = - canv.height * state.zoom * state.pY + state.mdownY;

            ctx.drawImage(image,  state.canvX, state.canvY, canv.width * state.zoom, canv.height * state.zoom)
        }
    })

    canv.addEventListener('mousedown', (event) => {
        state.button = event.button;
        state.mDown = true;
        var rect = canv.getBoundingClientRect();
            
        state.mdownX = event.clientX - rect.left;
        state.mdownY = event.clientY - rect.top;

        state.pX = (state.mdownX - state.canvX) / (canv.width * state.zoom);
        state.pY = (state.mdownY - state.canvY) / (canv.height * state.zoom);
    })

    canv.addEventListener('mouseup', (event) => {
        state.mDown = false;
    })

    canv.addEventListener('wheel', (event) => {
        state.mDown = false;
    })

    canv.addEventListener('dblclick', (event) => {
        
        //Uncomment this line for map calibration
        //console.log(state.pX + ',' + state.pY)

        const carlaX = 10482.4274 * state.pX + -5.39801455 * state.pY - 5673.07949;
        const carlaY = 5.39801455 * state.pX + 10482.4274 * state.pY - 2885.15738;
        coords[0].textContent = carlaX.toFixed(1)
        coords[1].textContent = carlaY.toFixed(1)
    })

})
</script>

