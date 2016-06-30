window.onload = function() {
    obj('ip').disabled = obj('dhcp').checked;
    obj('gw').disabled = obj('dhcp').checked;
    obj('mk').disabled = obj('dhcp').checked;
    obj('dhcp').onchange = function() {
        obj('ip').disabled = this.checked;
        obj('gw').disabled = this.checked;
        obj('mk').disabled = this.checked;
    }
}

function save() {
    if (!obj('dhcp')) {
        if (ip('ip')) return error('Invalid Local IP');
        if (ip('gw')) return error('Invalid Gateway');
        if (ip('mk')) return error('Invalid Subnet');
    }
}

function error(str) {
    obj('error').innerHTML = str;
    obj('error').style.display = 'block';
    return false;
}

function ip(id) {
    if (obj(id).value.search(/\b(([01]?\d?\d|2[0-4]\d|25[0-5])\.){3}([01]?\d?\d|2[0-4]\d|25[0-5])\b/)) return true;
}

function obj(id) {
    return document.getElementById(id);
}
