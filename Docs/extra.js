$(document).ready(function () {
    if(window.location.href.indexOf("latest") != -1) {
        var banner = document.createElement("div");
        banner.className = "dev-version-warning";
        banner.innerHTML = "You are currently reading documentation for the \"dev\" branch of CARLA. This documentation refers to features currently in development and may result in unexpected behaviour. To read documentation for previous releases, select the desired version in the bottom, right-hand corner of the screen.";
        var x = document.getElementsByClassName("section");
        console.log(x)
        x[0].insertBefore(banner,x[0].childNodes[0]);
    }
});