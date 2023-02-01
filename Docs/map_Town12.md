# Town 12

Town 12 is a Large Map with dimensions of 10x10 km<sup>2</sup>. It is divided into 25 tiles each with dimensions of 2x2 km<sup>2</sup>. The road layout is partially inspired by the road layout of the city of Amarillo in Texas, USA. There are numerous regions to the city including urban, residential and rural and a large highway system surrounding the city. 

## Navigator

The navigator interactive map can be used to browse the town and derive coordinates to use in the CARLA simulator.

__Using the navigator__:

* `left mouse button` - click and hold, drag downwards to zoom in, drag upwards to zoom out
* `middle mouse button` or `press mouse wheel` - click and hold, drag left, right, up or down to move the map


![town12_aerial](../img/maps/town12hyperres.webp#map)

<script>
window.addEventListener('load', function () {
  
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

        console.log(state)

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
        console.log('double click')
        
        const p = document.createElement("p")
        const textNode = document.createTextNode("Carla coordinate: X: " + (10 * state.mdownX).toFixed(1) + " Y: " + (10 * state.mdownY).toFixed(1));
        p.appendChild(textNode)

        if(state.dblClick){
            state.list.appendChild(p)
        } else {
            state.list = canv.parentNode.parentNode.appendChild(document.createElement('li'))
            state.list.appendChild(p)
            state.dblClick = true;
        }

        console.log(canv.parentNode.parentNode)
    })

})
</script>

