<div id="error"></div>
<form method="post">
    <div class="row">
        <div class="title">SSID</div>
        <select name="my_ssid">
            {networks}
        </select>
    </div>
    <div class="row">
        <div class="title">Password</div>
        <input type="text" name="my_password" value="{my_password}">
    </div>
    <div class="row">
        <div class="title">Host</div>
        <input type="text" name="io_host" value="{io_host}">
    </div>
    <div class="row">
        <div class="title">&nbsp;</div>
        <label><input type="checkbox" id="dhcp" name="dhcp" {dhcp_checked} value="1"> DHCP mode</label>
    </div>
    <div class="row">
        <div class="title">Local IP</div>
        <input type="text" id="ip" name="local_ip" value="{local_ip}">
    </div>
    <div class="row">
        <div class="title">Gateway</div>
        <input type="text" id="gw" name="gateway" value="{gateway}">
    </div>
    <div class="row">
        <div class="title">Subnet</div>
        <input type="text" id="mk" name="subnet" value="{subnet}">
    </div>
    <div class="row">
        <div class="title">&nbsp;</div>
        <button type="submit" onclick="return save();">OK</button>
    </div>
</form>
