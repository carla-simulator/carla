# Town 12

Town 12 is a Large Map with dimensions of 10x10 km<sup>2</sup>. It is divided into 25 tiles each with dimensions of 2x2 km<sup>2</sup>. The road layout is partially inspired by the road layout of the city of Amarillo in Texas, USA. There are numerous regions to the city including urban, residential and rural and a large highway system surrounding the city and the architectural styles reflect those of many medium to large cities across North America.  

## Navigator

The navigator interactive map can be used to browse the town and derive coordinates to use in the CARLA simulator.

__Using the navigator__:

* `left mouse button` - click and hold, drag downwards to zoom in, drag upwards to zoom out
* `middle mouse button` or `press mouse wheel` - click and hold, drag left, right, up or down to move the map
* `double click` - double click on a point on the map to record the coordinates, you will find the coordinates in the code block just below the map


![town12_aerial](../img/maps/town12hyperres.webp#map)

__CARLA coordinates__:

Copy and paste this code into a notebook or Python terminal to translate the spectator to the desired location:

```py
spectator = world.get_spectator()
loc = carla.Location(0.0, 0.0, 500.0)
rot = carla.Rotation(pitch=-90, yaw=0.0, roll=-90.0)
spectator.set_transform(carla.Transform(loc, rot))
```

## Town 12 zones

__High-rise downtown__:

Town 12's downtown area is a large span of high rise skyscrapers arranged into blocks on a consistent grid of roads, resembling downtown areas in many large American and European cities.

![high_rise](../img/maps/town12/high_rise.webp)

__High density residential__:

The high density residential areas of Town 12 have many 2-10 storey apartment buildings with commercial properties like cafes and retail stores at street level.

![high_dens_res](../img/maps/town12/high_dens_res.webp)

__Low density residential__:

The low density residential regions of Town 12 reflect the classic suburbs of many American cities, with one and two story homes surrounded by fenced gardens and garages.

![low_dens_res](../img/maps/town12/low_dens_res.webp)

__Highways and intersections__:

Town 12 has an extensive highway system, including 3-4 lane highways interspersed with impressive roundabout junctions.

![highway](../img/maps/town12/highway.webp)

__Rural and farmland__:

Town 12 also has rural regions with characteristic farmland buildings like wooden barns and farmhouses, windmills, grain silos, corn fields, hay bails and rural fencing. 

![rural](../img/maps/town12/rural.webp)

<script>
window.addEventListener('load', function () {

    const coords = document.getElementsByClassName("hljs-number")
    coords[0].textContent = 10
    console.log(coords[0].textContent)
  
    const q = document.querySelector('[src$="map"]');
    const canv = document.createElement('canvas');
    canv.setAttribute('height', q.height)
    canv.setAttribute('width', q.width)
    q.parentNode.replaceChild(canv, q)

    var state = {mDown: false, button: 0, lastX: 0, lastY:0, canvX: 0, canvY: 0, zoom: 1.0, mdownX: 0, mdownY: 0, pX: 0.5, pY: 0.5, dblClick: false, listObj: false}

    ctx = canv.getContext('2d')
    ctx.drawImage(q, 0, 0, canv.width, canv.height)

    canv.addEventListener('mousemove', (event) => {
        dX = event.clientX - state.lastX
        dY = event.clientY - state.lastY
        state.lastX = event.clientX
        state.lastY = event.clientY

        if(state.mDown && state.button == 1) {
            state.canvX += dX
            state.canvY += dY
            ctx.clearRect(0, 0, canv.width, canv.height)
            ctx.drawImage(q,  state.canvX, state.canvY, canv.width * state.zoom, canv.height * state.zoom)
        }
        if(state.mDown && state.button == 0) {
            if(dY > 0){
                state.zoom *= 1.05 
            } else {
               state.zoom *= 0.95
            }
            ctx.clearRect(0, 0, canv.width, canv.height)
            offX = -state.mdownX * (state.zoom - 1)
            offY = -state.mdownY * (state.zoom - 1)

            var rect = canv.getBoundingClientRect();
            
            var mX = event.clientX - rect.left;
            var mY = event.clientY - rect.top;

            state.canvX = - canv.width * state.zoom * state.pX + state.mdownX;
            state.canvY = - canv.height * state.zoom * state.pY + state.mdownY;
            //state.canvX += offX
            //state.canvY += offY
            ctx.drawImage(q,  state.canvX, state.canvY, canv.width * state.zoom, canv.height * state.zoom)
            //ctx.drawImage(q,  state.canvX, state.canvY, canv.width * state.zoom, canv.height * state.zoom)
            //ctx.drawImage(q,  state.canvX, state.canvY, canv.width * state.zoom, canv.height * state.zoom)
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
        console.log('Px: ' + state.pX + ' Py: ' + state.pY)
        
        const p = document.createElement("p")

        const carlaX = 10497.8366 * state.pX + 0.620390035 * state.pY - 5549.64759;
        const carlaY = 0.620390035 * state.pX + 10497.8366 * state.pY - 2898.42642;
        coords[0].textContent = carlaX.toFixed(1)
        coords[1].textContent = carlaY.toFixed(1)

        console.log(canv.parentNode.parentNode)
    })

})
</script>

