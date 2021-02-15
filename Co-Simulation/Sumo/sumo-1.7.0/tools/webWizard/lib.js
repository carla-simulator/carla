function elem(selector, attributes, events){
    if(selector[0] == "<"){
        selector = selector.substring(1, selector.length - 1);
        var node = document.createElement(selector);

        var i, j;
        if(attributes){
            for(i in attributes){
                if(attributes[i] instanceof Object){
                    for(j in attributes[i])
                        node[i][j] = attributes[i][j];
                } else
                    node[i] = attributes[i];
            }
        }

        if(events){
            for(i in events)
                node.addEventListener(i, events[i]);
        }

        return node;
    } else
        return document.querySelector(selector);
}

function elems(selector){
    return document.querySelectorAll(selector);
}

function on(type, callback){
    if(type == "ready")
        type = "DOMContentLoaded";

    addEventListener(type, callback);
}

HTMLElement.prototype.on = function(type, callback){
    this.addEventListener(type, callback);
};

HTMLElement.prototype.elem = function(r){
    return this.querySelector(r);
};

HTMLElement.prototype.append = function(node){
    if(typeof node == "string")
        node = elem.apply(null, arguments);

    return this.appendChild(node);
};

NodeList.prototype.forEach = Array.prototype.forEach;

function lequal(){
    for(var i = 1, l = arguments.length; i < l; ++i){
        if(arguments[i - 1] > arguments[i])
            return false;
    }

    return true;
}
