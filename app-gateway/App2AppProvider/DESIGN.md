App2AppProvider design overview

- Follows Thunder plugin architecture:
  - Implements PluginHost::IPlugin for lifecycle (Initialize/Deinitialize/Information).
  - Exposes JSON-RPC using PluginHost::JSONRPC with versioned interface "org.rdk.App2AppProvider.1".
  - Uses Register/Unregister for method bindings; Notify for event emissions.

- Configuration schema:
  - enabled: boolean
  - providerId: string
  - capabilities: array<string>

- Events:
  - statechanged (payload: state string)
  - appregistered (payload: appId string)
  - appunregistered (payload: appId string)
  - message (payload: { from, to, payload })

- Minimal in-memory app registry using std::set<string> with thread safety via CriticalSection.

- Error handling:
  - Returns Core::ERROR_BAD_REQUEST for missing parameters.
  - Returns Core::ERROR_NONE on success.

- CMake integration:
  - CMakeLists.txt follows AppGateway pattern, links WPEFrameworkCore/Plugins/Tracing.
  - Installs plugin library and config.

- Future work:
  - Integrate with AppGateway routing tables per appgateway-technical-design.md.
  - Authentication/authorization for message send.
  - Persistence and subscription filters.
