on("ready", function(){
    var PORT = "8010";

    /**
     * @class
     * @param display: the display string
     * @param internal: the internal string, like the VClass string in SUMO
     * @param fringeFactor: initial fringe factor value
     * @param count: initial count value
     */
    function Settings(){
        this.init.apply(this, arguments);
    }

    Settings.prototype = {
        init: function(display, internal, fringeFactor, count, enabled){
            this.internal = internal;

            var node = elem("<div>", {className: "container"});
            var header = elem("<h4>", {textContent: display});
            header.append("<img>", {src: "images/" + internal + ".png"});
            node.append(header);
            this.enable = elem("<input>", {type: "checkbox", checked: enabled});
            node.append(this.enable);

            var options = elem("<div>", {className: "options"});
            var label = elem("<label>", {textContent: "Through Traffic Factor"});
            this.fringeFactor = elem("<input>", {type: "number", min: .5, max: 100, step: .1, value: fringeFactor});
            label.append(this.fringeFactor);
            options.append(label);

            label = elem("<label>", {textContent: "Count", title: "Count per hour per kilometer"});
            this.count = elem("<input>", {type: "number", min: .2, max: 100, step: .1, value: count});
            label.append(this.count);
            options.append(label);

            node.append(options);
            elem("#vehicle-controls").append(node);
        },

        toJSON: function(){
            if(this.enable.checked){
                return {
                    fringeFactor: parseFloat(this.fringeFactor.value),
                    count: parseFloat(this.count.value)
                };
            }

            return null;
        }
    };

    var vehicleClasses = [
        new Settings("Cars", "passenger", 5, 12, true),
        new Settings("Trucks", "truck", 5, 8),
        new Settings("Bus", "bus", 5, 4),
        new Settings("Motorcycles", "motorcycle", 2, 4),
        new Settings("Bicycles", "bicycle", 2, 6),
        new Settings("Pedestrians", "pedestrian", 1, 10),
        new Settings("Trams", "tram", 20, 2),
        new Settings("Urban trains", "rail_urban", 40, 2),
        new Settings("Trains", "rail", 40, 2),
        new Settings("Ships", "ship", 40, 2)
    ];

    var activeTab = null;

    /**
     * @function
     * @param id: the id of the tab to open
     */
    function openTab(id){
        var tab = elems(".tab")[id];
        var side = elem("#side");
        var control = elems(".controls")[id];

        // clicked on the open tab, close everything
        if(activeTab === id){
            side.classList.remove("open");
            control.classList.remove("open");
            tab.classList.remove("open");
            activeTab = null;
        } else {
            // open the side and control
            side.classList.add("open");
            control.classList.add("open");
            tab.classList.add("open");

            // close the other tab, if there is one
            if(activeTab !== null){
                elems(".controls")[activeTab].classList.remove("open");
                elems(".tab")[activeTab].classList.remove("open");
            }

            activeTab = id;
        }
    }

    elems(".tab").forEach(function(tab, index){
        tab.dataset.symbol = tab.textContent;
        tab.on("click", function(){
            openTab(index);
        });
    });

    openTab(0);

    var canvas = elem("canvas");
    var canvasActive = false;
    var canvasRect = [.1, .1, .9, .9];
    var ctx = canvas.getContext("2d");

    /**
     * @function
     * sets the canvas to full page size
     */
    function setCanvasSize(){
        canvas.width = innerWidth;
        canvas.height = innerHeight;
        draw();
    }

    setCanvasSize();
    on("resize", setCanvasSize);

    /**
     * @function
     * draws the rect on the canvas, to select an area
     **/
    function draw(){
        var x0 = canvas.width * canvasRect[0],
            y0 = canvas.height * canvasRect[1],
            x1 = canvas.width * canvasRect[2] - 250,
            y1 = canvas.height * canvasRect[3];

        ctx.clearRect(0, 0, canvas.width, canvas.height);

        ctx.fillStyle = "#808080";
        ctx.globalAlpha = .5;

        ctx.fillRect(0, 0, canvas.width, canvas.height);
        ctx.clearRect(x0, y0, x1 - x0, y1 - y0);
    }

    var mouse = {
        x: 0, // last x coordinate
        y: 0, // last y coordinates
        area: null // if mouse button is down, and in which function; move or resize
    }

    canvas.on("mousedown", function(evt){
        mouse.area = changeMousePointer(evt.clientX, evt.clientY, true);
    });
    canvas.on("mouseup", function(evt){
        mouse.area = null;
        changeMousePointer(evt.clientX, evt.clientY);
    });

    function changeMousePointer(x, y, down){
        var x0 = canvas.width * canvasRect[0],
            y0 = canvas.height * canvasRect[1],
            x1 = canvas.width * canvasRect[2] - 250,
            y1 = canvas.height * canvasRect[3];

        var cursor = "", t = 20; //tolerance

        if(lequal(x0 - t, x, x1 + t) && lequal(y0 - t, y, y1 + t)){
            if(lequal(y0 - t, y, y0 + t))
                cursor += "n";
            else if(lequal(y1 - t, y, y1 + t))
                cursor += "s";

            if(lequal(x0 - t, x, x0 + t))
                cursor += "w";
            else if(lequal(x1 - t, x, x1 + t))
                cursor += "e";

            if(cursor) cursor += "-resize";
            else if(mouse.area || down) cursor = "move";
            else cursor = "pointer";
        } else
            cursor = "auto"

        canvas.style.cursor = cursor;
        return cursor.match(/^([nsm])?([we])?/);
    }

    canvas.on("mousemove", function(evt){
        changeMousePointer(evt.clientX, evt.clientY);
        var dx = (evt.clientX - mouse.x) / canvas.width, dy = (evt.clientY - mouse.y) / canvas.height;
        mouse.x = evt.clientX;
        mouse.y = evt.clientY;
        if(mouse.area !== null){
            if(mouse.area[1] == "n")
                canvasRect[1] += dy;
            else if(mouse.area[1] == "s")
                canvasRect[3] += dy;

            if(mouse.area[2] == "w")
                canvasRect[0] += dx;
            else if(mouse.area[2] == "e")
                canvasRect[2] += dx;

            if(mouse.area[1] == "m"){
                canvasRect[0] += dx;
                canvasRect[1] += dy;
                canvasRect[2] += dx;
                canvasRect[3] += dy;
            }

            draw();
        }
    });

    /**
     * @function
     * checks if the checkbox is checked and displays the canvas in this case, else not
     **/
    function toogleCanvas(){
        if(canvasToogle.checked){
            canvasActive = true;
            canvas.style.removeProperty("display");
        } else {
            canvasActive = false;
            canvas.style.display = "none";
        }
    }

    var canvasToogle = elem("#canvas-toogle");

    canvasToogle.on("click", toogleCanvas);
    toogleCanvas();

    // OSM map
    // avoid cross domain resource sharing issues (#3991)
    // (https://gis.stackexchange.com/questions/83953/openlayers-maps-issue-with-ssl)
    var map = new OpenLayers.Map("map");
    var maplayer = new OpenLayers.Layer.OSM("OpenStreetMap", 
    // Official OSM tileset as protocol-independent URLs
    [
        'https://a.tile.openstreetmap.org/${z}/${x}/${y}.png',
        'https://b.tile.openstreetmap.org/${z}/${x}/${y}.png',
        'https://c.tile.openstreetmap.org/${z}/${x}/${y}.png'
    ], null);
    map.addLayer(maplayer);

    function setPosition(lon, lat){
        if(!lon || !lat){
            latLon = elem("#lat_lon").value.split(" ")
            lon = parseFloat(latLon[1]);
            lat = parseFloat(latLon[0]);
        } else {
            elem("#lat_lon").value = lat.toFixed(6) + " " + lon.toFixed(6);
        }

        var leftHandBounds = [new OpenLayers.Bounds(-9, 50, 3, 60), // British Isles
                              new OpenLayers.Bounds(66, 3, 90, 30), // India, Pakistan
                              new OpenLayers.Bounds(95, -45, 179, 2), // Australia, Indonesia
                              new OpenLayers.Bounds(-20, -35, 40, -15), // Southern Africa
                              new OpenLayers.Bounds(135, 30, 150, 42), // Japan
                             ];
        elem("#leftHand").checked = false;
        for (var i = 0; i < leftHandBounds.length; i++) {
            if (leftHandBounds[i].contains(lon, lat)) {
                elem("#leftHand").checked = true;
                break;
            }
        }

        var lonLat = new OpenLayers.LonLat(lon, lat);
        lonLat.transform(
            new OpenLayers.Projection("EPSG:4326"), // transform from WGS 1984
            map.getProjectionObject() // to Spherical Mercator Projection
        );

        map.setCenter(lonLat, 16);
    }

    function setPositionByString() {
      query = elem("#address").value
      var url = "https://nominatim.openstreetmap.org/search?q=" + query + "&format=json&polygon=0&addressdetails=0&limit=1&callback";
      getJSON(url,
          function(err, data) {
            if (err != null) {
              window.alert('Could not locate address: ' + err);
            } else if (data.length == 0) {
              window.alert('Could not locate address');
            } else {
              var result = data[0];
              lon = parseFloat(result.lon);
              lat = parseFloat(result.lat);
              setPosition(lon, lat);
            }
          });
    }

    elem("#address").on("keyup", function(e){
        if (e.keyCode == 13) {
            setPositionByString();
        }
    });

    var getJSON = function(url, callback) {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', url, true);
        xhr.responseType = 'json';
        xhr.onload = function() {
          var status = xhr.status;
          if (status == 200) {
            callback(null, xhr.response);
          } else {
            callback(status);
          }
        };
        xhr.send();
    };
    

    // set default position to center Berlin
    setPosition(13.4, 52.52);

    /**
     * @listener
     * set the coordinates of the map to current coordinates (got from browser)
     */
    elem("#buttonCurrent").on("click", function(){
        if(!navigator.geolocation) return;

        navigator.geolocation.getCurrentPosition(function(position){
            setPosition(position.coords.longitude, position.coords.latitude);
        });
    });

    /**
     * @listener
     * whenever the input boxes changes, update the map coordinates
     */
    elem("#buttonSearch").on("click", setPositionByString);
    elem("#buttonLatLon").on("click", setPosition);

    /**
     * @listener
     * whenever the map coordinates changes, update the input boxes
     */
    map.events.register("move", map, function(){
        var cor = map.getExtent();
        cor.transform(
            map.getProjectionObject(), // from Spherical Mercator Projection
            new OpenLayers.Projection("EPSG:4326")
        );
        lat = (cor.top + (cor.bottom - cor.top) / 2);
        lon = (cor.left + (cor.right - cor.left) / 2);

        elem("#lat_lon").value = lat.toFixed(6) + " " + lon.toFixed(6);
    });

    var socket;
    var totalSteps;
    var currentStep;
    var presentedErrorLog = false;

    /**
     * @function
     * connects to the socket, when it fails it tries it again after five seconds
     */
    function connectSocket(){
        var address = location.hostname;
        // when accessing via file, location.hostname is an empty string, so guess that the server is on localhost
        if(!address)
            address = "localhost";
        try {
            socket = new WebSocket("ws://" + address + ":" + PORT);
        } catch(e){
            // connection failed, wait five seconds, then try again
	    setTimeout(connectSocket, 5000);
            return;
        }

	socket.onerror = function(error) {
	    if (presentedErrorLog == false) {
		window.alert("Socket connection failed. Please open the OSM WebWizard by using osmWebWizard.py or the link in your start menu.");
		presentedErrorLog = true;
	    }
	};
	
        // whenever the socket closes (e.g. restart) try to reconnect
        socket.addEventListener("close", connectSocket);
        socket.addEventListener("message", function(evt){
            var message = evt.data;
            // get the first space
            var index = message.indexOf(" ");
            // split the message type from the message
            var type = message.substr(0, index);
            message = message.substr(index + 1);

            if(type === "zip"){
                showZip(message);
            } else if(type === "report"){
                currentStep++;
                elem("#status > span").textContent = message;
                elem("#status > div").style.width = (100 * currentStep / totalSteps) + "%";

                if(currentStep === totalSteps){
                    setTimeout(function(){
                        elem("#status").style.display = "none";
                    elem("#export-button").style.display = "block";
                    }, 2000);
                }
            } else if(type === "steps"){
                totalSteps = parseInt(message);
                currentStep = 0;
            }
        });
    }

    connectSocket();

    /**
     * @function
     * generate and send the data to the websocket
     */
    function startBuild(){
        var cor = map.getExtent();
        cor.transform(
            map.getProjectionObject(), // from Spherical Mercator Projection
            new OpenLayers.Projection("EPSG:4326")
        );

        var data = {
            poly: elem("#polygons").checked,
            duration: parseInt(elem("#duration").value),
            publicTransport: elem("#publicTransport").checked,
            leftHand: elem("#leftHand").checked,
            decal: elem("#decal").checked,
            carOnlyNetwork: elem("#carOnlyNetwork").checked,
            vehicles: {}
        };

        // calculates the coordinates of the rectangle if area-picking is active
        if(canvasActive){
            var width = cor.right - cor.left;
            var height = cor.bottom - cor.top;
            data.coords = [
                cor.left + width * canvasRect[0],
                cor.top + height * canvasRect[3],
                cor.left + width * canvasRect[2],
                cor.top + height * canvasRect[1]
            ];
        } else
            data.coords = [cor.left, cor.bottom, cor.right, cor.top];

        vehicleClasses.forEach(function(vehicleClass){
            var result = vehicleClass.toJSON();
            if(result)
                data.vehicles[vehicleClass.internal] = result;
        });

        try {
            socket.send(JSON.stringify(data));
        } catch(e){
            return;
        }

        elem("#status").style.display = "block";
        elem("#export-button").style.display = "none";
    }

    elem("#export-button").on("click", startBuild);

    /**
     * @function
     * @param uri: a base64-encoded string for a zip file
     * shows a download dialog to save the zip file
     */
    function showZip(uri){
        var url = "data:application/zip;base64," + uri;

        // using a temporarily link to trigger the download dialog
        var link = elem("<a>", {
            download: "osm.zip",
            href: url,
            target: "_blank"
        });

        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);
    }
});
