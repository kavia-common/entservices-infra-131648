#include "AppGateway.h"
#include <core/Enumerate.h>

namespace WPEFramework {
namespace Plugin {

    namespace {
        static Plugin::Metadata<Plugin::AppGateway> metadata(
            // Registration with JSON-RPC interface version
            AppGateway::ServiceName(),
            AppGateway::Version,
            // Description
            _T("AppGateway Thunder plugin to route application requests."),
            // Autostart default
            false,
            // Activatable
            true);
    }

    AppGateway::AppGateway()
        : PluginHost::JSONRPC()
        , _service(nullptr)
        , _config()
        , _state(_T("Uninitialized"))
    {
        RegisterAll();
    }

    AppGateway::~AppGateway()
    {
        UnregisterAll();
    }

    const string AppGateway::Initialize(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);
        _service = service;

        // Read configuration from JSON
        string message;

        if (service != nullptr) {
            _config.FromString(service->ConfigLine());

            // initial state
            _state = _config.Enabled.IsSet() && _config.Enabled.Value() ? _T("Enabled") : _T("Disabled");
        } else {
            message = _T("Service shell is null");
        }

        return message;
    }

    void AppGateway::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(service == _service);
        Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
        _service = nullptr;
        _state = _T("Deinitialized");
    }

    string AppGateway::Information() const
    {
        Core::JSON::String info;
        info = _T("AppGateway v") + Core::NumberType<uint32_t>(Version).Text();
        return info.Value();
    }

    void AppGateway::RegisterAll()
    {
        Register<Core::JSON::String, Core::JSON::String>(_T("ping"),
            [this](const Core::JSON::String&, Core::JSON::String& response) {
                return endpoint_ping(response);
            });

        Register<Core::JSON::Void, Core::JSON::Object>(_T("getInfo"),
            [this](const Core::JSON::Void&, Core::JSON::Object& response) {
                return endpoint_getInfo(response);
            });

        Register<Core::JSON::Object, Core::JSON::Object>(_T("setConfig"),
            [this](const Core::JSON::Object& params, Core::JSON::Object& response) {
                return endpoint_setConfig(params, response);
            });
    }

    void AppGateway::UnregisterAll()
    {
        Unregister(_T("ping"));
        Unregister(_T("getInfo"));
        Unregister(_T("setConfig"));
    }

    uint32_t AppGateway::endpoint_ping(Core::JSON::String& response)
    {
        response = _T("pong");
        return Core::ERROR_NONE;
    }

    uint32_t AppGateway::endpoint_getInfo(Core::JSON::Object& response)
    {
        // Provide minimal info payload
        Core::JSON::String name;
        name = APPGATEWAY_PLUGIN_NAME;

        Core::JSON::String state;
        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            state = _state.c_str();
        }

        response.Set(_T("name"), name);
        response.Set(_T("version"), Core::JSON::DecUInt8(Version));
        response.Set(_T("state"), state);

        return Core::ERROR_NONE;
    }

    uint32_t AppGateway::endpoint_setConfig(const Core::JSON::Object& params, Core::JSON::Object& response)
    {
        // Validate and set known keys
        Core::JSON::Boolean enabled;
        Core::JSON::String routeTable;

        if (params.HasLabel(_T("enabled"))) {
            enabled = params.Get<Core::JSON::Boolean>(_T("enabled"));
            _config.Enabled = enabled;
        }
        if (params.HasLabel(_T("routeTable"))) {
            routeTable = params.Get<Core::JSON::String>(_T("routeTable"));
            _config.RouteTable = routeTable;
        }

        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            _state = _config.Enabled.IsSet() && _config.Enabled.Value() ? _T("Enabled") : _T("Disabled");
        }

        // Emit event
        event_statechanged(Core::JSON::String(_state));

        // Echo back current config
        response.Set(_T("enabled"), _config.Enabled);
        response.Set(_T("routeTable"), _config.RouteTable);

        return Core::ERROR_NONE;
    }

    void AppGateway::event_statechanged(const Core::JSON::String& state)
    {
        // Broadcast JSON-RPC notification
        Notify(_T("statechanged"), state);
    }

} // namespace Plugin
} // namespace WPEFramework
