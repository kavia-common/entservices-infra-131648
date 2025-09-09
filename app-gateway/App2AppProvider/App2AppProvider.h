#pragma once

#include "Module.h"

#include <core/JSON.h>
#include <plugins/Plugin.h>
#include <plugins/JSONRPC.h>

namespace WPEFramework {
namespace Plugin {

/**
 * PUBLIC_INTERFACE
 * App2AppProvider Thunder plugin provides app-to-app messaging interface
 * and discovery services for applications via JSON-RPC.
 *
 * JSON-RPC interface: "org.rdk.App2AppProvider.1"
 * Methods:
 *  - ping() -> "pong"
 *  - getInfo() -> { name, version, state }
 *  - configure({ enabled, providerId, capabilities }) -> echoes config
 *  - registerApp({ appId }) -> { registered: true|false }
 *  - unregisterApp({ appId }) -> { unregistered: true|false }
 *  - sendMessage({ from, to, payload }) -> { delivered: true|false }
 *  - listApps() -> { apps: [appId...] }
 * Events:
 *  - statechanged: { "state": "<Enabled|Disabled|...>" }
 *  - appregistered: { "appId": "<id>" }
 *  - appunregistered: { "appId": "<id>" }
 *  - message: { "from": "<id>", "to": "<id>", "payload": "<string>" }
 */
class App2AppProvider : public PluginHost::IPlugin, public PluginHost::JSONRPC {
public:
    class Config : public Core::JSON::Container {
    public:
        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;

        Config()
            : Core::JSON::Container()
        {
            Add(_T("enabled"), &Enabled);
            Add(_T("providerId"), &ProviderId);
            Add(_T("capabilities"), &Capabilities);
        }

        Core::JSON::Boolean Enabled;
        Core::JSON::String ProviderId;
        Core::JSON::ArrayType<Core::JSON::String> Capabilities;
    };

public:
    App2AppProvider(const App2AppProvider&) = delete;
    App2AppProvider& operator=(const App2AppProvider&) = delete;

    // PUBLIC_INTERFACE
    App2AppProvider();
    ~App2AppProvider() override;

    // IPlugin
    // PUBLIC_INTERFACE
    const string Initialize(PluginHost::IShell* service) override;
    // PUBLIC_INTERFACE
    void Deinitialize(PluginHost::IShell* service) override;
    // PUBLIC_INTERFACE
    string Information() const override;

    // JSON-RPC Methods
    // PUBLIC_INTERFACE
    void RegisterAll();
    // PUBLIC_INTERFACE
    void UnregisterAll();

    // Versioned interface name for JSON-RPC
    static constexpr const TCHAR* ServiceName() { return _T("org.rdk.App2AppProvider"); }
    static constexpr const uint8_t Version = 1;

private:
    // JSON-RPC method handlers
    uint32_t endpoint_ping(Core::JSON::String& response);
    uint32_t endpoint_getInfo(Core::JSON::Object& response);
    uint32_t endpoint_configure(const Core::JSON::Object& params, Core::JSON::Object& response);
    uint32_t endpoint_registerApp(const Core::JSON::Object& params, Core::JSON::Object& response);
    uint32_t endpoint_unregisterApp(const Core::JSON::Object& params, Core::JSON::Object& response);
    uint32_t endpoint_sendMessage(const Core::JSON::Object& params, Core::JSON::Object& response);
    uint32_t endpoint_listApps(Core::JSON::Object& response);

    // Event helpers
    void event_statechanged(const Core::JSON::String& state);
    void event_appregistered(const Core::JSON::String& appId);
    void event_appunregistered(const Core::JSON::String& appId);
    void event_message(const Core::JSON::Object& message);

    // internal helpers
    bool RegisterAppInternal(const string& appId);
    bool UnregisterAppInternal(const string& appId);
    bool IsRegistered(const string& appId) const;

private:
    PluginHost::IShell* _service;
    Config _config;
    mutable Core::CriticalSection _adminLock;

    // Example internal state
    string _state;

    // rudimentary in-memory registry of apps
    std::set<string> _apps;
};

} // namespace Plugin
} // namespace WPEFramework
