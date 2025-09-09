#include "App2AppProvider.h"

#include <set>

namespace WPEFramework {
namespace Plugin {

    namespace {
        static Plugin::Metadata<Plugin::App2AppProvider> metadata(
            // Registration with JSON-RPC interface version
            App2AppProvider::ServiceName(),
            App2AppProvider::Version,
            // Description
            _T("App2AppProvider Thunder plugin to provide app-to-app discovery and messaging."),
            // Autostart default
            false,
            // Activatable
            true);
    }

    App2AppProvider::App2AppProvider()
        : PluginHost::JSONRPC()
        , _service(nullptr)
        , _config()
        , _state(_T("Uninitialized"))
        , _apps()
    {
        RegisterAll();
    }

    App2AppProvider::~App2AppProvider()
    {
        UnregisterAll();
    }

    const string App2AppProvider::Initialize(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);
        _service = service;

        // Read configuration from JSON
        string message;

        if (service != nullptr) {
            _config.FromString(service->ConfigLine());

            // initial state
            {
                Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
                _state = _config.Enabled.IsSet() && _config.Enabled.Value() ? _T("Enabled") : _T("Disabled");
            }

            // Emit lifecycle state event on initialization
            event_statechanged(Core::JSON::String(_state));
        } else {
            message = _T("Service shell is null");
        }

        return message;
    }

    void App2AppProvider::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(service == _service);

        // Update state to Disabled for deactivation and notify clients
        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            _state = _T("Disabled");
        }

        // Emit lifecycle state event while plugin is still active
        event_statechanged(Core::JSON::String(_state));

        // Cleanup internal state
        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            _apps.clear();
            _service = nullptr;
        }
    }

    string App2AppProvider::Information() const
    {
        Core::JSON::String info;
        info = _T("App2AppProvider v") + Core::NumberType<uint32_t>(Version).Text();
        return info.Value();
    }

    void App2AppProvider::RegisterAll()
    {
        Register<Core::JSON::String, Core::JSON::String>(_T("ping"),
            [this](const Core::JSON::String&, Core::JSON::String& response) {
                return endpoint_ping(response);
            });

        Register<Core::JSON::Void, Core::JSON::Object>(_T("getInfo"),
            [this](const Core::JSON::Void&, Core::JSON::Object& response) {
                return endpoint_getInfo(response);
            });

        Register<Core::JSON::Object, Core::JSON::Object>(_T("configure"),
            [this](const Core::JSON::Object& params, Core::JSON::Object& response) {
                return endpoint_configure(params, response);
            });

        Register<Core::JSON::Object, Core::JSON::Object>(_T("registerApp"),
            [this](const Core::JSON::Object& params, Core::JSON::Object& response) {
                return endpoint_registerApp(params, response);
            });

        Register<Core::JSON::Object, Core::JSON::Object>(_T("unregisterApp"),
            [this](const Core::JSON::Object& params, Core::JSON::Object& response) {
                return endpoint_unregisterApp(params, response);
            });

        Register<Core::JSON::Object, Core::JSON::Object>(_T("sendMessage"),
            [this](const Core::JSON::Object& params, Core::JSON::Object& response) {
                return endpoint_sendMessage(params, response);
            });

        Register<Core::JSON::Void, Core::JSON::Object>(_T("listApps"),
            [this](const Core::JSON::Void&, Core::JSON::Object& response) {
                return endpoint_listApps(response);
            });
    }

    void App2AppProvider::UnregisterAll()
    {
        Unregister(_T("ping"));
        Unregister(_T("getInfo"));
        Unregister(_T("configure"));
        Unregister(_T("registerApp"));
        Unregister(_T("unregisterApp"));
        Unregister(_T("sendMessage"));
        Unregister(_T("listApps"));
    }

    uint32_t App2AppProvider::endpoint_ping(Core::JSON::String& response)
    {
        response = _T("pong");
        return Core::ERROR_NONE;
    }

    uint32_t App2AppProvider::endpoint_getInfo(Core::JSON::Object& response)
    {
        // Provide minimal info payload
        Core::JSON::String name;
        name = APP2APPPROVIDER_PLUGIN_NAME;

        Core::JSON::String state;
        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            state = _state.c_str();
        }

        response.Set(_T("name"), name);
        response.Set(_T("version"), Core::JSON::DecUInt8(Version));
        response.Set(_T("state"), state);
        response.Set(_T("providerId"), _config.ProviderId);
        response.Set(_T("capabilities"), _config.Capabilities);

        return Core::ERROR_NONE;
    }

    uint32_t App2AppProvider::endpoint_configure(const Core::JSON::Object& params, Core::JSON::Object& response)
    {
        Core::JSON::Boolean enabled;
        Core::JSON::String providerId;
        Core::JSON::ArrayType<Core::JSON::String> caps;

        // Validate and apply 'enabled' (optional, boolean)
        if (params.HasLabel(_T("enabled"))) {
            enabled = params.Get<Core::JSON::Boolean>(_T("enabled"));
            _config.Enabled = enabled;
        }

        // Validate and apply 'providerId' (optional, must be a non-empty string if present)
        if (params.HasLabel(_T("providerId"))) {
            providerId = params.Get<Core::JSON::String>(_T("providerId"));
            if (providerId.Value().empty()) {
                Core::JSON::String msg;
                msg = _T("Validation error: 'providerId' must be a non-empty string.");
                response.Set(_T("message"), msg);
                return Core::ERROR_GENERAL;
            }
            _config.ProviderId = providerId;
        }

        // Validate and apply 'capabilities' (optional, must be an array-of-strings if present)
        if (params.HasLabel(_T("capabilities"))) {
            caps = params.Get<Core::JSON::ArrayType<Core::JSON::String>>(_T("capabilities"));

            // Iterate to ensure elements are strings; ArrayType<Core::JSON::String> enforces type,
            // but we further ensure entries are not null-like.
            bool invalid = false;
            Core::JSON::ArrayType<Core::JSON::String>::Iterator index(caps.Elements());
            while (index.Next() == true) {
                const Core::JSON::String& v = index.Current();
                // Type is string by template; allow empty strings but ensure no null/undefined
                // If stricter behavior is desired (no empty strings), uncomment the block below.
                // if (v.Value().empty()) { invalid = true; break; }
                if (!v.IsSet()) {
                    invalid = true;
                    break;
                }
            }

            if (invalid) {
                Core::JSON::String msg;
                msg = _T("Validation error: 'capabilities' must be an array of strings.");
                response.Set(_T("message"), msg);
                return Core::ERROR_GENERAL;
            }

            _config.Capabilities = caps;
        }

        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            _state = _config.Enabled.IsSet() && _config.Enabled.Value() ? _T("Enabled") : _T("Disabled");
        }

        // Emit event
        event_statechanged(Core::JSON::String(_state));

        // Echo back current config
        response.Set(_T("enabled"), _config.Enabled);
        response.Set(_T("providerId"), _config.ProviderId);
        response.Set(_T("capabilities"), _config.Capabilities);

        return Core::ERROR_NONE;
    }

    uint32_t App2AppProvider::endpoint_registerApp(const Core::JSON::Object& params, Core::JSON::Object& response)
    {
        // Enforce enabled/disabled gating: only proceed if enabled
        bool enabled = false;
        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            enabled = (_state == _T("Enabled"));
        }
        if (!enabled) {
            Core::JSON::String msg;
            msg = _T("Service unavailable: App2AppProvider is disabled");
            response.Set(_T("message"), msg);
            return Core::ERROR_UNAVAILABLE;
        }

        Core::JSON::String appId;

        // Validate required param presence
        if (!params.HasLabel(_T("appId"))) {
            Core::JSON::String msg;
            msg = _T("Validation error: 'appId' is required and must be a non-empty string.");
            response.Set(_T("message"), msg);
            return Core::ERROR_GENERAL;
        }

        appId = params.Get<Core::JSON::String>(_T("appId"));

        // Validate non-empty string
        if (appId.Value().empty()) {
            Core::JSON::String msg;
            msg = _T("Validation error: 'appId' must be a non-empty string.");
            response.Set(_T("message"), msg);
            return Core::ERROR_GENERAL;
        }

        bool ok = RegisterAppInternal(appId.Value());
        Core::JSON::Boolean registered;
        registered = ok;
        response.Set(_T("registered"), registered);
        if (ok) {
            event_appregistered(appId);
        }
        return Core::ERROR_NONE;
    }

    uint32_t App2AppProvider::endpoint_unregisterApp(const Core::JSON::Object& params, Core::JSON::Object& response)
    {
        // Enforce enabled/disabled gating: only proceed if enabled
        bool enabled = false;
        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            enabled = (_state == _T("Enabled"));
        }
        if (!enabled) {
            Core::JSON::String msg;
            msg = _T("Service unavailable: App2AppProvider is disabled");
            response.Set(_T("message"), msg);
            return Core::ERROR_UNAVAILABLE;
        }

        Core::JSON::String appId;

        // Validate required param presence
        if (!params.HasLabel(_T("appId"))) {
            Core::JSON::String msg;
            msg = _T("Validation error: 'appId' is required and must be a non-empty string.");
            response.Set(_T("message"), msg);
            return Core::ERROR_GENERAL;
        }

        appId = params.Get<Core::JSON::String>(_T("appId"));

        // Validate non-empty string
        if (appId.Value().empty()) {
            Core::JSON::String msg;
            msg = _T("Validation error: 'appId' must be a non-empty string.");
            response.Set(_T("message"), msg);
            return Core::ERROR_GENERAL;
        }

        bool ok = UnregisterAppInternal(appId.Value());
        Core::JSON::Boolean unregistered;
        unregistered = ok;
        response.Set(_T("unregistered"), unregistered);
        if (ok) {
            event_appunregistered(appId);
        }
        return Core::ERROR_NONE;
    }

    uint32_t App2AppProvider::endpoint_sendMessage(const Core::JSON::Object& params, Core::JSON::Object& response)
    {
        // Enforce enabled/disabled gating: only proceed if enabled
        bool enabled = false;
        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            enabled = (_state == _T("Enabled"));
        }
        if (!enabled) {
            Core::JSON::String msg;
            msg = _T("Service unavailable: App2AppProvider is disabled");
            response.Set(_T("message"), msg);
            return Core::ERROR_UNAVAILABLE;
        }

        Core::JSON::String from;
        Core::JSON::String to;
        Core::JSON::String payload;

        // Validate required labels
        if (!params.HasLabel(_T("from")) || !params.HasLabel(_T("to")) || !params.HasLabel(_T("payload"))) {
            Core::JSON::String msg;
            msg = _T("Validation error: 'from', 'to', and 'payload' are required. 'from' and 'to' must be non-empty strings; 'payload' must be a string.");
            response.Set(_T("message"), msg);
            return Core::ERROR_GENERAL;
        }

        from = params.Get<Core::JSON::String>(_T("from"));
        to = params.Get<Core::JSON::String>(_T("to"));
        payload = params.Get<Core::JSON::String>(_T("payload"));

        // Validate appId strings (must be non-empty)
        if (from.Value().empty()) {
            Core::JSON::String msg;
            msg = _T("Validation error: 'from' must be a non-empty string.");
            response.Set(_T("message"), msg);
            return Core::ERROR_GENERAL;
        }
        if (to.Value().empty()) {
            Core::JSON::String msg;
            msg = _T("Validation error: 'to' must be a non-empty string.");
            response.Set(_T("message"), msg);
            return Core::ERROR_GENERAL;
        }

        // Validate registered apps (in this minimal build, allow if 'to' is registered)
        bool deliverable = IsRegistered(to.Value());
        Core::JSON::Boolean delivered;
        delivered = deliverable;
        response.Set(_T("delivered"), delivered);

        if (deliverable) {
            Core::JSON::Object evt;
            evt.Set(_T("from"), from);
            evt.Set(_T("to"), to);
            evt.Set(_T("payload"), payload);
            event_message(evt);
        }

        return Core::ERROR_NONE;
    }

    uint32_t App2AppProvider::endpoint_listApps(Core::JSON::Object& response)
    {
        // Enforce enabled/disabled gating: only proceed if enabled
        bool enabled = false;
        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            enabled = (_state == _T("Enabled"));
        }
        if (!enabled) {
            Core::JSON::String msg;
            msg = _T("Service unavailable: App2AppProvider is disabled");
            response.Set(_T("message"), msg);
            return Core::ERROR_UNAVAILABLE;
        }

        Core::JSON::ArrayType<Core::JSON::String> apps;
        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            for (const auto& id : _apps) {
                Core::JSON::String v;
                v = id;
                apps.Add(v);
            }
        }
        response.Set(_T("apps"), apps);
        return Core::ERROR_NONE;
    }

    void App2AppProvider::event_statechanged(const Core::JSON::String& state)
    {
        Notify(_T("statechanged"), state);
    }

    void App2AppProvider::event_appregistered(const Core::JSON::String& appId)
    {
        Notify(_T("appregistered"), appId);
    }

    void App2AppProvider::event_appunregistered(const Core::JSON::String& appId)
    {
        Notify(_T("appunregistered"), appId);
    }

    void App2AppProvider::event_message(const Core::JSON::Object& message)
    {
        Notify(_T("message"), message);
    }

    bool App2AppProvider::RegisterAppInternal(const string& appId)
    {
        if (appId.empty()) {
            return false;
        }
        Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
        auto result = _apps.insert(appId);
        return result.second;
    }

    bool App2AppProvider::UnregisterAppInternal(const string& appId)
    {
        Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
        return _apps.erase(appId) > 0;
    }

    bool App2AppProvider::IsRegistered(const string& appId) const
    {
        Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
        return _apps.find(appId) != _apps.end();
    }

} // namespace Plugin
} // namespace WPEFramework
