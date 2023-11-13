# Town 13

![town_13](../img/catalogue/maps/town13/town13montage.webp)

Town 13 is a Large Map with dimensions of 10x10 km<sup>2</sup>. It is divided into 36 tiles, most with dimensions of 2x2 km<sup>2</sup> (some edge tiles are smaller). There are numerous contrasting regions to the city including urban, residential and rural areas, along with a large highway system surrounding the city with a ringroad. The architectural styles reflect those of many medium to large cities across North America.  

!!! note
    Town 13 has been designed as an adjunct to Town 13, such that they can serve as a __train-test pair__. The towns share many common features, but also many differences in the styles of buildings, road textures, pavement textures and also vegetation. Using one of the pair to produce training data and then using the other for testing is ideal for exposing overfitting issues that might be arising while developing an AD stack.

## Navigator

The navigator interactive map can be used to browse the town and derive coordinates to use in the CARLA simulator.

__Using the navigator__:

* `left mouse button` - click and hold, drag left, right, up or down to move the map
* `scroll mouse wheel` - scroll down to zoom out, scroll up to zoom in on the location under the mouse pointer
* `double click` - double click on a point on the map to record the coordinates, you will find the coordinates in the text and the code block just below the map

__Zone color reference__:

* <span style="color:#595d5e; background-color:#595d5e;">&nbsp</span>   [Skyscraper](#high-rise-downtown)
* <span style="color:#d2dddc; background-color:#d2dddc;">&nbsp</span>   [High density residential](#high-density-residential)
* <span style="color:#838c8b; background-color:#838c8b;">&nbsp</span>   [Community buildings](#community-buildings)
* <span style="color:#17d894; background-color:#17d894;">&nbsp</span>   [Low density residential](#low-density-residential)
* <span style="color:#df6a19; background-color:#df6a19;">&nbsp</span>   [Parks](#parks)
* <span style="color:#839317; background-color:#839317;">&nbsp</span>   [Rural farmland](#rural-and-farmland)
* <span style="color:#265568; background-color:#265568;">&nbsp</span>   [Water](#water)



![town13_aerial](../img/catalogue/maps/town13/town13roadrunner.webp#map)


__CARLA coordinates__: 

* __X__:  <span id="carlacoord_x" style="animation: fadeMe 2s;">--</span>
* __Y__:  <span id="carlacoord_y" style="animation: fadeMe 2s;">--</span>


After double clicking on a point of interest, the navigator will display the corresponding CARLA coordinates and update them in the following code block. Copy and paste the code into a notebook or Python terminal to translate the spectator to the desired location. You will need first to [connect the client and set up the world object](tuto_first_steps.md#launching-carla-and-connecting-the-client):

```py
# CARLA coordinates: X 0.0, Y 0.0
spectator = world.get_spectator()
loc = carla.Location(0.0, 0.0, 300.0)
rot = carla.Rotation(pitch=-90, yaw=0.0, roll=0.0)
spectator.set_transform(carla.Transform(loc, rot))
```
## Town 13 zones

#### High-rise downtown:

Town 13's downtown area is a large span of high-rise skyscrapers arranged into blocks on a consistent grid of roads, resembling downtown areas in many large American and European cities.

![high_rise](../img/catalogue/maps/town13/high_rise.webp)

#### Community buildings:

The community buildings are a set of 2-4 storey apartment buildings in a colorful bohemian style with businesses on the ground floors, located next to the downtown area of the city.

![community](../img/catalogue/maps/town13/community.webp)

#### High density residential:

The high density residential areas of Town 13 have many 2-10 storey apartment buildings with commercial properties like cafes and retail stores at street level. Many of the residential blocks have balconies with sun shades similar to those in sunny southern European countries.

![high_dens_res](../img/catalogue/maps/town13/high_dens_res.webp)

#### Low density residential:

The low density residential regions of Town 13 reflect the suburbs of many European cities, with one and two story homes surrounded by fenced gardens and garages.

![low_dens_res](../img/catalogue/maps/town13/low_dens_res.webp)

#### Parks:

The dense residential and downtown areas are broken up by small islands of green communal space, juxtaposing green foliage against urban architecture.

![parks](../img/catalogue/maps/town13/parks.webp)

#### Highways and intersections:

Town 13 has an extensive highway system, including 3-4 lane highways, large roundabouts and a causeway over a large body of water.

![highway](../img/catalogue/maps/town13/highway.webp)

#### Rural and farmland:

Town 13 also has rural regions with characteristic farmland buildings like wooden barns and farmhouses, windmills, grain silos, corn fields, hay bails and rural fencing. These areas have unmarked country dirt roads and single lane interurban roads for inter-city traffic.

![rural](../img/catalogue/maps/town13/rural.webp)

#### Water:

There are several bodies of water in town 13 including a large lake with a central island and several ponds in rural areas. 

![water](../img/catalogue/maps/town13/water.webp)

<style>
@keyframes fadeMe {
  from {
    color: #77aaff;
  }
  to {
    color: #000000;
  }
}

</style>
<script>
window.addEventListener('load', function () {

    var text_coord_x = document.getElementById("carlacoord_x")
    var text_coord_y = document.getElementById("carlacoord_y")
    const code_coords = document.getElementsByClassName("hljs-number")
    const code_comment = document.getElementsByClassName("hljs-comment")
  
    const image = document.querySelector('[src$="map"]');
    const canv = document.createElement('canvas');

    canv.setAttribute('height', image.height)
    canv.setAttribute('width', image.width)
    image.parentNode.replaceChild(canv, image)

    var state = {mDown: false, button: 0, lastX: 0, lastY:0, canvX: 0, canvY: 0, zoom: 1.0, mdownX: 0, mdownY: 0, pX: 0.5, pY: 0.5, dblClick: false, listObj: false, touch: false}

    ctx = canv.getContext('2d')
    ctx.drawImage(image, 0, 0, canv.width, canv.height)

    canv.addEventListener('mousemove', (event) => {
        dX = event.clientX - state.lastX
        dY = event.clientY - state.lastY
        state.lastX = event.clientX
        state.lastY = event.clientY

        if(state.mDown && state.button == 0) {
            state.canvX += dX
            state.canvY += dY
            ctx.clearRect(0, 0, canv.width, canv.height)
            ctx.drawImage(image,  state.canvX, state.canvY, canv.width * state.zoom, canv.height * state.zoom)
            state.touch = true;
        }
    })

    canv.addEventListener('mousedown', (event) => {

        state.button = event.button;
        state.mDown = true;
        state.touch = true;

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

        var rect = canv.getBoundingClientRect();

        dX = event.clientX - rect.left;
        dY = event.clientY - rect.top;

        state.pX = (dX - state.canvX) / (canv.width * state.zoom);
        state.pY = (dY - state.canvY) / (canv.height * state.zoom);

        if(state.touch){
            event.preventDefault();
            if(event.wheelDelta > 0){
                state.zoom *= 1.15 
            } else {
               state.zoom *= 0.85
            }

            if(state.zoom < 1.0){state.zoom = 1.0;}
            if(state.zoom > 30.0){state.zoom = 30.0}

            ctx.clearRect(0, 0, canv.width, canv.height)

            state.canvX = - canv.width * state.zoom * state.pX + dX;
            state.canvY = - canv.height * state.zoom * state.pY + dY;

            ctx.drawImage(image,  state.canvX, state.canvY, canv.width * state.zoom, canv.height * state.zoom);
        }
        
    })

    canv.addEventListener('dblclick', (event) => {
        
        text_coord_x = document.getElementById("carlacoord_x")
        text_coord_y = document.getElementById("carlacoord_y")

        const carlaX = 14556.0868 * state.pX + -7.34734913 * state.pY - 6655.00941;
        const carlaY = 2.19186383 * state.pX + 12431.3323 * state.pY - 4524.46039;

        code_coords[0].textContent = carlaX.toFixed(1)
        code_coords[1].textContent = carlaY.toFixed(1)
        code_comment[0].textContent = "# CARLA coordinates - X: " + carlaX.toFixed(1) + " Y: " + carlaY.toFixed(1)

        var newX = text_coord_x.cloneNode(true)
        var newY = text_coord_y.cloneNode(true)

        newX.textContent = carlaX.toFixed(1)
        newY.textContent = carlaY.toFixed(1)

        var parentX = text_coord_x.parentNode
        var parentY = text_coord_y.parentNode

        parentX.replaceChild(newX, text_coord_x);
        parentY.replaceChild(newY, text_coord_y);

        //console.log(state.pX + ", " + state.pY)

    })

})
</script>

