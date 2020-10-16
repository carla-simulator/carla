

function ButtonAction(container_name){
  if(window_big){
    snipet_name = container_name.replace('-snipet_button','-snipet');
    document.getElementById("snipets-container").innerHTML = document.getElementById(snipet_name).innerHTML;
  }
  else{
    document.getElementById("snipets-container").innerHTML = null;
    code_name = container_name.replace('-snipet_button','-python');
    alert(code_name);
    var range = document.createRange();
    range.selectNode(document.getElementById(code_name));
    alert(range);
  }
}

  function WindowResize(){
  if(window.innerWidth > 1200){
    window_big = true;
  }
  else{
    window_big = false;
  }
}

var window_big;
if(window.innerWidth > 1200){
    window_big = true;
  }
else{
    window_big = false;
  }
buttons = document.getElementsByClassName('SnipetButton')
for (let i = 0; i < buttons.length; i++) {
    buttons[i].addEventListener("click",function(){ButtonAction(buttons[i].id);},true);
  }

window.onresize = WindowResize;