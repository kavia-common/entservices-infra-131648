#pragma once

#include "Module.h"

#include <core/JSON.h>
#include <plugins/Plugin.h>
#include <plugins/JSONRPC.h>

namespace WPEFramework {
namespace Plugin {

/**
 * PUBLIC_INTERFACE
 * AppGateway Thunder plugin provides a simple gateway to manage and route
 * application-related requests to underlying managers/services.
 *
 * JSON-RPC interface: "org.rdk.AppGateway.1"
 * Methods:
 *  - ping()
 *  - getInfo()
 *  - setConfig(params)
 * Events:
 *  - statechanged
 */
class AppGateway : public PluginHost::IPlugin, public PluginHost::JSONRPC {
public:
    class Config : public Core::JSON::Container {
    public:
        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;

        Config()
            : Core::JSON::Container()
        {
            Add(_T("enabled"), &Enabled);
            Add(_T("routeTable"), &RouteTable);
        }

        Core::JSON::Boolean Enabled;
        Core::JSON::String RouteTable;
    };

public:
    AppGateway(const AppGateway&) = delete;
    AppGateway& operator=(const AppGateway&) = delete;

    // PUBLIC_INTERFACE
    AppGateway();
    ~AppGateway() override;

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
    static constexpr const TCHAR* ServiceName() { return _T("org.rdk.AppGateway"); }
    static constexpr const uint8_t Version = 1;

private:
    // JSON-RPC method handlers
    uint32_t endpoint_ping(Core::JSON::String& response);
    uint32_t endpoint_getInfo(Core::JSON::Object& response);
    uint32_t endpoint_setConfig(const Core::JSON::Object& params, Core::JSON::Object& response);

    // Event helpers
    void event_statechanged(const Core::JSON::String& state);

private:
    PluginHost::IShell* _service;
    Config _config;
    mutable Core::CriticalSection _adminLock;

    // Example internal state
    string _state;
};

} // namespace Plugin
} // namespace WPEFramework
