

// Snipet container div functions to copy and clean the content

function CopyToClipboard(containerid) {
  if (document.selection) {
  var range = document.body.createTextRange();
  range.moveToElementText(document.getElementById(containerid));
  range.select().createTextRange();
  document.execCommand("copy");
  } 
  else if (window.getSelection) {
  var range = document.createRange();
  range.selectNode(document.getElementById(containerid));
  window.getSelection().addRange(range);
  document.execCommand("copy");
  }
}

function CloseSnipet() {
  document.getElementById("snipets-container").innerHTML = null;
}