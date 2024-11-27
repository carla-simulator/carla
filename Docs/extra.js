$(document).ready(function () {
    if(window.location.href.indexOf("/latest/") != -1) {
        var banner = document.createElement("div");
        var section = document.getElementsByClassName("section");
        banner.className = "dev-version-warning";
        banner.innerHTML = "You are currently reading documentation for the \"ue5-dev\" branch of the Unreal Engine 5 version of CARLA. The functionality referred to in this documentation may exhibit instability or unexpected behavior.";
        section[0].insertBefore(banner,section[0].childNodes[0]);
    }

    var nav = document.getElementsByClassName("wy-side-nav-search")
    var img = document.createElement("img");
    img.src = "https://raw.githubusercontent.com/carla-simulator/carla/359f723d20edd1084512c967a53e22771c9a1045/Docs/img/logos/carla_ue5_logo.png";
    img.style.width = "100%"
    img.style.borderRadius = "0px"
    img.style.height = img.style.width
    img.style.backgroundColor = "#c7e9fd"
    nav[0].insertBefore(img, nav[0].childNodes[2])
    
});