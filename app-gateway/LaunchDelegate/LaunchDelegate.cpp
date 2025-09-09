#include "LaunchDelegate.h"

#include <core/Enumerate.h>

namespace WPEFramework {
namespace Plugin {

    namespace {
        static Plugin::Metadata<Plugin::LaunchDelegate> metadata(
            // Registration with JSON-RPC interface version
            LaunchDelegate::ServiceName(),
            LaunchDelegate::Version,
            // Description
            _T("LaunchDelegate Thunder plugin to delegate app lifecycle operations."),
            // Autostart default
            false,
            // Activatable
            true);
    }

    LaunchDelegate::LaunchDelegate()
        : PluginHost::JSONRPC()
        , _service(nullptr)
        , _config()
        , _state(_T("Uninitialized"))
        , _statusByAppId()
    {
        RegisterAll();
    }

    LaunchDelegate::~LaunchDelegate()
    {
        UnregisterAll();
    }

    const string LaunchDelegate::Initialize(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);
        _service = service;

        string message;

        if (service != nullptr) {
            _config.FromString(service->ConfigLine());

            // initial state
            {
                Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
                _state = _config.Enabled.IsSet() && _config.Enabled.Value() ? _T("Enabled") : _T("Disabled");
            }
        } else {
            message = _T("Service shell is null");
        }

        return message;
    }

    void LaunchDelegate::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(service == _service);
        Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
        _service = nullptr;
        _state = _T("Deinitialized");
        _statusByAppId.clear();
    }

    string LaunchDelegate::Information() const
    {
        Core::JSON::String info;
        info = _T("LaunchDelegate v") + Core::NumberType<uint32_t>(Version).Text();
        return info.Value();
    }

    void LaunchDelegate::RegisterAll()
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

        Register<Core::JSON::Object, Core::JSON::Object>(_T("launch"),
            [this](const Core::JSON::Object& params, Core::JSON::Object& response) {
                return endpoint_launch(params, response);
            });

        Register<Core::JSON::Object, Core::JSON::Object>(_T("stop"),
            [this](const Core::JSON::Object& params, Core::JSON::Object& response) {
                return endpoint_stop(params, response);
            });

        Register<Core::JSON::Object, Core::JSON::Object>(_T("status"),
            [this](const Core::JSON::Object& params, Core::JSON::Object& response) {
                return endpoint_status(params, response);
            });
    }

    void LaunchDelegate::UnregisterAll()
    {
        Unregister(_T("ping"));
        Unregister(_T("getInfo"));
        Unregister(_T("configure"));
        Unregister(_T("launch"));
        Unregister(_T("stop"));
        Unregister(_T("status"));
    }

    uint32_t LaunchDelegate::endpoint_ping(Core::JSON::String& response)
    {
        response = _T("pong");
        return Core::ERROR_NONE;
    }

    uint32_t LaunchDelegate::endpoint_getInfo(Core::JSON::Object& response)
    {
        Core::JSON::String name;
        name = LAUNCHDELEGATE_PLUGIN_NAME;

        Core::JSON::String state;
        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            state = _state.c_str();
        }

        response.Set(_T("name"), name);
        response.Set(_T("version"), Core::JSON::DecUInt8(Version));
        response.Set(_T("state"), state);
        response.Set(_T("defaultDelegate"), _config.DefaultDelegate);

        Core::JSON::Object timeouts;
        timeouts.Set(_T("launchMs"), _config.Timeouts.LaunchMs);
        timeouts.Set(_T("stopMs"), _config.Timeouts.StopMs);
        response.Set(_T("timeouts"), timeouts);

        return Core::ERROR_NONE;
    }

    uint32_t LaunchDelegate::endpoint_configure(const Core::JSON::Object& params, Core::JSON::Object& response)
    {
        if (params.HasLabel(_T("enabled"))) {
            _config.Enabled = params.Get<Core::JSON::Boolean>(_T("enabled"));
        }
        if (params.HasLabel(_T("defaultDelegate"))) {
            _config.DefaultDelegate = params.Get<Core::JSON::String>(_T("defaultDelegate"));
        }
        if (params.HasLabel(_T("timeouts"))) {
            auto timeoutsObj = params.Get<Core::JSON::Object>(_T("timeouts"));
            if (timeoutsObj.HasLabel(_T("launchMs"))) {
                _config.Timeouts.LaunchMs = timeoutsObj.Get<Core::JSON::DecUInt32>(_T("launchMs"));
            }
            if (timeoutsObj.HasLabel(_T("stopMs"))) {
                _config.Timeouts.StopMs = timeoutsObj.Get<Core::JSON::DecUInt32>(_T("stopMs"));
            }
        }

        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            _state = _config.Enabled.IsSet() && _config.Enabled.Value() ? _T("Enabled") : _T("Disabled");
        }

        // Emit event
        event_statechanged(Core::JSON::String(_state));

        // Echo back current config
        response.Set(_T("enabled"), _config.Enabled);
        response.Set(_T("defaultDelegate"), _config.DefaultDelegate);

        Core::JSON::Object timeouts;
        timeouts.Set(_T("launchMs"), _config.Timeouts.LaunchMs);
        timeouts.Set(_T("stopMs"), _config.Timeouts.StopMs);
        response.Set(_T("timeouts"), timeouts);

        return Core::ERROR_NONE;
    }

    uint32_t LaunchDelegate::endpoint_launch(const Core::JSON::Object& params, Core::JSON::Object& response)
    {
        if (!IsEnabled()) {
            return Core::ERROR_ILLEGAL_STATE;
        }

        if (!params.HasLabel(_T("appId"))) {
            return Core::ERROR_BAD_REQUEST;
        }

        const auto appId = params.Get<Core::JSON::String>(_T("appId")).Value();
        const string requestId = GenerateRequestId();

        // fire launchrequested event
        {
            Core::JSON::Object evt;
            evt.Set(_T("appId"), Core::JSON::String(appId));
            evt.Set(_T("requestId"), Core::JSON::String(requestId));
            event_launchrequested(evt);
        }

        string error;
        bool ok = DelegateLaunch(appId, params, error);

        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            auto& st = _statusByAppId[appId];
            st.LastUpdated = Core::Time::Now();
            st.State = ok ? _T("Launching") : _T("Error");
            st.LastError = ok ? string() : error;
        }

        // fire completion event
        {
            Core::JSON::Object evt;
            evt.Set(_T("appId"), Core::JSON::String(appId));
            evt.Set(_T("requestId"), Core::JSON::String(requestId));
            Core::JSON::Boolean success; success = ok;
            evt.Set(_T("success"), success);
            if (!ok) {
                evt.Set(_T("error"), Core::JSON::String(error));
            }
            event_launchcompleted(evt);
        }

        Core::JSON::Boolean accepted; accepted = true;
        response.Set(_T("accepted"), accepted);
        response.Set(_T("requestId"), Core::JSON::String(requestId));
        return Core::ERROR_NONE;
    }

    uint32_t LaunchDelegate::endpoint_stop(const Core::JSON::Object& params, Core::JSON::Object& response)
    {
        if (!IsEnabled()) {
            return Core::ERROR_ILLEGAL_STATE;
        }

        if (!params.HasLabel(_T("appId"))) {
            return Core::ERROR_BAD_REQUEST;
        }

        const auto appId = params.Get<Core::JSON::String>(_T("appId")).Value();
        const string requestId = GenerateRequestId();

        // fire stoprequested event
        {
            Core::JSON::Object evt;
            evt.Set(_T("appId"), Core::JSON::String(appId));
            evt.Set(_T("requestId"), Core::JSON::String(requestId));
            event_stoprequested(evt);
        }

        string error;
        bool ok = DelegateStop(appId, params, error);

        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            auto& st = _statusByAppId[appId];
            st.LastUpdated = Core::Time::Now();
            st.State = ok ? _T("Stopping") : _T("Error");
            st.LastError = ok ? string() : error;
        }

        // fire completion event
        {
            Core::JSON::Object evt;
            evt.Set(_T("appId"), Core::JSON::String(appId));
            evt.Set(_T("requestId"), Core::JSON::String(requestId));
            Core::JSON::Boolean success; success = ok;
            evt.Set(_T("success"), success);
            if (!ok) {
                evt.Set(_T("error"), Core::JSON::String(error));
            }
            event_stopcompleted(evt);
        }

        Core::JSON::Boolean accepted; accepted = true;
        response.Set(_T("accepted"), accepted);
        response.Set(_T("requestId"), Core::JSON::String(requestId));
        return Core::ERROR_NONE;
    }

    uint32_t LaunchDelegate::endpoint_status(const Core::JSON::Object& params, Core::JSON::Object& response)
    {
        if (!params.HasLabel(_T("appId"))) {
            return Core::ERROR_BAD_REQUEST;
        }

        const auto appId = params.Get<Core::JSON::String>(_T("appId")).Value();

        Core::JSON::String state; state = _T("Unknown");
        Core::JSON::String lastError;
        Core::JSON::DecUInt32 pid; pid = 0;

        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            auto it = _statusByAppId.find(appId);
            if (it != _statusByAppId.end()) {
                state = it->second.State.c_str();
                if (!it->second.LastError.empty()) {
                    lastError = it->second.LastError.c_str();
                }
                pid = it->second.Pid;
            }
        }

        response.Set(_T("state"), state);
        if (lastError.Length() > 0) {
            response.Set(_T("lastError"), lastError);
        }
        response.Set(_T("pid"), pid);
        return Core::ERROR_NONE;
    }

    void LaunchDelegate::event_statechanged(const Core::JSON::String& state)
    {
        Notify(_T("statechanged"), state);
    }

    void LaunchDelegate::event_launchrequested(const Core::JSON::Object& payload)
    {
        Notify(_T("launchrequested"), payload);
    }

    void LaunchDelegate::event_launchcompleted(const Core::JSON::Object& payload)
    {
        Notify(_T("launchcompleted"), payload);
    }

    void LaunchDelegate::event_stoprequested(const Core::JSON::Object& payload)
    {
        Notify(_T("stoprequested"), payload);
    }

    void LaunchDelegate::event_stopcompleted(const Core::JSON::Object& payload)
    {
        Notify(_T("stopcompleted"), payload);
    }

    string LaunchDelegate::GenerateRequestId() const
    {
        // Generate a simple pseudo-UUID using time ticks; sufficient as request correlation id
        auto now = Core::Time::Now().Ticks();
        return Core::NumberType<uint64_t>(now).Text();
    }

    bool LaunchDelegate::IsEnabled() const
    {
        Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
        return _config.Enabled.IsSet() && _config.Enabled.Value();
    }

    bool LaunchDelegate::DelegateLaunch(const string& appId, const Core::JSON::Object& params, string& error)
    {
        // Placeholder delegation: in a full implementation, this would route to AppGateway/LifecycleManager/AppManager per design.
        // Here, we pretend delegation is accepted.
        Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
        auto& st = _statusByAppId[appId];
        st.State = _T("Launching");
        st.LastError.clear();
        st.Pid = 0;
        st.LastUpdated = Core::Time::Now();
        return true;
    }

    bool LaunchDelegate::DelegateStop(const string& appId, const Core::JSON::Object& params, string& error)
    {
        // Placeholder delegation: accept stop in this thin layer.
        Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
        auto it = _statusByAppId.find(appId);
        if (it != _statusByAppId.end()) {
            it->second.State = _T("Stopping");
            it->second.LastError.clear();
            it->second.LastUpdated = Core::Time::Now();
        }
        return true;
    }

    void LaunchDelegate::UpdateStateLocked(const string& s)
    {
        Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
        _state = s;
    }

} // namespace Plugin
} // namespace WPEFramework
