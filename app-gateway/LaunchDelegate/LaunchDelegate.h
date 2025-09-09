#pragma once

#include "Module.h"

#include <core/JSON.h>
#include <plugins/Plugin.h>
#include <plugins/JSONRPC.h>

namespace WPEFramework {
namespace Plugin {

/**
 * PUBLIC_INTERFACE
 * LaunchDelegate Thunder plugin provides an abstraction to delegate application
 * launch/terminate/inspect requests to the appropriate underlying manager/gateway.
 *
 * JSON-RPC interface: "org.rdk.LaunchDelegate.1"
 * Methods:
 *  - ping() -> "pong"
 *  - getInfo() -> { name, version, state }
 *  - configure({ enabled, defaultDelegate, timeouts: { launchMs, stopMs } }) -> echoes config
 *  - launch({ appId, parameters?, intent?, caller? }) -> { accepted: true, requestId }
 *  - stop({ appId, reason? }) -> { accepted: true, requestId }
 *  - status({ appId }) -> { state, lastError?, pid? }
 * Events:
 *  - statechanged: { "state": "<Enabled|Disabled|...>" }
 *  - launchrequested: { "appId": "<id>", "requestId": "<uuid>" }
 *  - launchcompleted: { "appId": "<id>", "requestId": "<uuid>", "success": true|false, "error"?: "<string>" }
 *  - stoprequested: { "appId": "<id>", "requestId": "<uuid>" }
 *  - stopcompleted: { "appId": "<id>", "requestId": "<uuid>", "success": true|false, "error"?: "<string>" }
 */
class LaunchDelegate : public PluginHost::IPlugin, public PluginHost::JSONRPC {
public:
    class Config : public Core::JSON::Container {
    public:
        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;

        Config()
            : Core::JSON::Container()
        {
            Add(_T("enabled"), &Enabled);
            Add(_T("defaultDelegate"), &DefaultDelegate);
            Add(_T("timeouts"), &Timeouts);
        }

        class Timeouts : public Core::JSON::Container {
        public:
            Timeouts()
                : Core::JSON::Container()
            {
                Add(_T("launchMs"), &LaunchMs);
                Add(_T("stopMs"), &StopMs);
            }
            Core::JSON::DecUInt32 LaunchMs;
            Core::JSON::DecUInt32 StopMs;
        };

        Core::JSON::Boolean Enabled;
        Core::JSON::String DefaultDelegate; // e.g., "AppGateway", "LifecycleManager"
        Timeouts Timeouts;
    };

public:
    LaunchDelegate(const LaunchDelegate&) = delete;
    LaunchDelegate& operator=(const LaunchDelegate&) = delete;

    // PUBLIC_INTERFACE
    LaunchDelegate();
    ~LaunchDelegate() override;

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
    static constexpr const TCHAR* ServiceName() { return _T("org.rdk.LaunchDelegate"); }
    static constexpr const uint8_t Version = 1;

private:
    // JSON-RPC method handlers
    uint32_t endpoint_ping(Core::JSON::String& response);
    uint32_t endpoint_getInfo(Core::JSON::Object& response);
    uint32_t endpoint_configure(const Core::JSON::Object& params, Core::JSON::Object& response);
    uint32_t endpoint_launch(const Core::JSON::Object& params, Core::JSON::Object& response);
    uint32_t endpoint_stop(const Core::JSON::Object& params, Core::JSON::Object& response);
    uint32_t endpoint_status(const Core::JSON::Object& params, Core::JSON::Object& response);

    // Event helpers
    void event_statechanged(const Core::JSON::String& state);
    void event_launchrequested(const Core::JSON::Object& payload);
    void event_launchcompleted(const Core::JSON::Object& payload);
    void event_stoprequested(const Core::JSON::Object& payload);
    void event_stopcompleted(const Core::JSON::Object& payload);

    // internal helpers
    string GenerateRequestId() const;
    bool IsEnabled() const;
    bool DelegateLaunch(const string& appId, const Core::JSON::Object& params, string& error);
    bool DelegateStop(const string& appId, const Core::JSON::Object& params, string& error);
    void UpdateStateLocked(const string& s);

private:
    PluginHost::IShell* _service;
    Config _config;
    mutable Core::CriticalSection _adminLock;

    // Example internal state
    string _state;

    // Minimal local cache for status
    struct AppStatus {
        Core::Time LastUpdated;
        string State; // e.g., "Launching", "Running", "Stopping", "Stopped", "Error"
        string LastError;
        uint32_t Pid { 0 };
    };
    std::map<string, AppStatus> _statusByAppId;
};

} // namespace Plugin
} // namespace WPEFramework
