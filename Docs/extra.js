$(document).ready(function () {
    if(window.location.href.indexOf("/latest/") != -1) {
        var banner = document.createElement("div");
        var section = document.getElementsByClassName("section");
        banner.className = "dev-version-warning";
        banner.innerHTML = "You are currently reading documentation for the \"dev\" branch of CARLA. This documentation refers to features currently in development and may result in unexpected behaviour. To read documentation for previous releases, select the desired version in the bottom, right-hand corner of the screen.";
        section[0].insertBefore(banner,section[0].childNodes[0]);
    }
    console.log('Running!')
    var nav = document.getElementsByClassName("wy-side-nav-search")
    const node = document.createTextNode("This is new.");
    console.log(nav[0].childNodes[0])
    console.log(nav[0].childNodes[1])
    console.log(nav[0].childNodes[2])

    var img = document.createElement("img");
    img.src = "carla_ue5_logo.png";
    img.style.width = "100%"
    img.style.borderRadius = "0px"
    img.style.height = img.style.width
    img.style.backgroundColor = "#c7e9fd"
    nav[0].insertBefore(img, nav[0].childNodes[2])
    
});