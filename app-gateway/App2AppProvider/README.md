# App2AppProvider Thunder Plugin

Provides a JSON-RPC interface to offer app-to-app discovery and messaging.

- Callsign: App2AppProvider
- Interface: org.rdk.App2AppProvider.1
- Methods:
  - ping() -> "pong"
  - getInfo() -> { name, version, state, providerId, capabilities }
  - configure({ enabled, providerId, capabilities: [str] }) -> echoes current config
  - registerApp({ appId }) -> { registered: true|false }
  - unregisterApp({ appId }) -> { unregistered: true|false }
  - sendMessage({ from, to, payload }) -> { delivered: true|false }
  - listApps() -> { apps: [appId...] }
- Events:
  - statechanged: { "state": "<Enabled|Disabled|...>" }
  - appregistered: { "appId": "<id>" }
  - appunregistered: { "appId": "<id>" }
  - message: { "from": "<id>", "to": "<id>", "payload": "<string>" }

Configuration (App2AppProvider.config):
{
  "callsign": "App2AppProvider",
  "locator": "libApp2AppProvider.so",
  "classname": "App2AppProvider",
  "autostart": false,
  "configuration": {
    "enabled": true,
    "providerId": "default-provider",
    "capabilities": ["messaging", "discovery"]
  }
}

Notes:
- For initial minimal implementation, messages are only "delivered" if the 'to' app is registered with the provider.
- This plugin aligns with Thunder plugin best practices: clear lifecycle, JSON-RPC registration, synchronous error codes (Core::ERROR_*), and event broadcasting with Notify().
- Extend with persistence, authentication, and routing per appgateway-technical-design.md as needed in future iterations.
