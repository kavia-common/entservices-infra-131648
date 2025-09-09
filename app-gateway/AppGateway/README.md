# AppGateway Thunder Plugin

Provides a JSON-RPC interface to manage and route application-related requests.

- Callsign: AppGateway
- Interface: org.rdk.AppGateway.1
- Methods:
  - ping() -> "pong"
  - getInfo() -> { name, version, state }
  - setConfig({ enabled, routeTable }) -> echoes current config
- Events:
  - statechanged: { "state": "<Enabled|Disabled|...>" }

Configuration (AppGateway.config):
{
  "callsign": "AppGateway",
  "locator": "libAppGateway.so",
  "classname": "AppGateway",
  "AUTOSTART": false,
  "configuration": {
    "enabled": true,
    "routeTable": ""
  }
}
