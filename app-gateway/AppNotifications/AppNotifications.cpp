#include "AppNotifications.h"
#include <core/Enumerate.h>

namespace WPEFramework {
namespace Plugin {

    namespace {
        static Plugin::Metadata<Plugin::AppNotifications> metadata(
            AppNotifications::ServiceName(),
            AppNotifications::Version,
            _T("AppNotifications Thunder plugin to broker app notifications."),
            false,
            true);
    }

    AppNotifications::AppNotifications()
        : PluginHost::JSONRPC()
        , _service(nullptr)
        , _config()
    {
        RegisterAll();
    }

    AppNotifications::~AppNotifications()
    {
        UnregisterAll();
    }

    const string AppNotifications::Initialize(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);
        _service = service;

        string message;

        if (service != nullptr) {
            _config.FromString(service->ConfigLine());

            // Defaults if not set
            if (!_config.MaxQueue.IsSet())
                _config.MaxQueue = static_cast<uint16_t>(64);
            if (!_config.AllowPublishWithoutSubscription.IsSet())
                _config.AllowPublishWithoutSubscription = false;
        } else {
            message = _T("Service shell is null");
        }

        return message;
    }

    void AppNotifications::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(service == _service);
        Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
        _service = nullptr;
        _subscriptions.clear();
        _retained.clear();
    }

    string AppNotifications::Information() const
    {
        Core::JSON::String info;
        info = _T("AppNotifications v") + Core::NumberType<uint32_t>(Version).Text();
        return info.Value();
    }

    void AppNotifications::RegisterAll()
    {
        Register<Core::JSON::String, Core::JSON::String>(_T("ping"),
            [this](const Core::JSON::String&, Core::JSON::String& response) {
                return endpoint_ping(response);
            });

        Register<Core::JSON::Void, Core::JSON::Object>(_T("getInfo"),
            [this](const Core::JSON::Void&, Core::JSON::Object& response) {
                return endpoint_getInfo(response);
            });

        Register<Types::SubscriptionRequest, Types::StatusResponse>(_T("subscribe"),
            [this](const Types::SubscriptionRequest& request, Types::StatusResponse& response) {
                return endpoint_subscribe(request, response);
            });

        Register<Core::JSON::String, Types::StatusResponse>(_T("unsubscribe"),
            [this](const Core::JSON::String& appId, Types::StatusResponse& response) {
                return endpoint_unsubscribe(appId, response);
            });

        Register<Types::NotificationMessage, Types::StatusResponse>(_T("publish"),
            [this](const Types::NotificationMessage& message, Types::StatusResponse& response) {
                return endpoint_publish(message, response);
            });

        Register<Core::JSON::Void, Core::JSON::ArrayType<Core::JSON::Object>>(_T("listSubscriptions"),
            [this](const Core::JSON::Void&, Core::JSON::ArrayType<Core::JSON::Object>& response) {
                return endpoint_listSubscriptions(response);
            });
    }

    void AppNotifications::UnregisterAll()
    {
        Unregister(_T("ping"));
        Unregister(_T("getInfo"));
        Unregister(_T("subscribe"));
        Unregister(_T("unsubscribe"));
        Unregister(_T("publish"));
        Unregister(_T("listSubscriptions"));
    }

    uint32_t AppNotifications::endpoint_ping(Core::JSON::String& response)
    {
        response = _T("pong");
        return Core::ERROR_NONE;
    }

    uint32_t AppNotifications::endpoint_getInfo(Core::JSON::Object& response)
    {
        Core::JSON::String name;
        name = APPNOTIFICATIONS_PLUGIN_NAME;

        Core::JSON::DecUInt8 version;
        version = Version;

        Core::JSON::DecUInt16 maxQueue;
        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            maxQueue = _config.MaxQueue.Value();
        }

        response.Set(_T("name"), name);
        response.Set(_T("version"), version);
        response.Set(_T("maxQueue"), maxQueue);

        return Core::ERROR_NONE;
    }

    uint32_t AppNotifications::endpoint_subscribe(const Types::SubscriptionRequest& request, Types::StatusResponse& response)
    {
        if (!request.AppId.IsSet() || request.AppId.Value().empty()) {
            response.Success = false;
            response.Message = _T("Missing appId");
            return Core::ERROR_BAD_REQUEST;
        }

        Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);

        Subscription sub;
        sub.appId = request.AppId.Value();
        sub.types.clear();
        if (request.Types.Length() > 0) {
            for (uint16_t i = 0; i < request.Types.Length(); ++i) {
                const Core::JSON::String& t = request.Types[i];
                if (t.IsSet() && !t.Value().empty()) {
                    sub.types.insert(t.Value());
                }
            }
        }

        _subscriptions[sub.appId] = sub;

        response.Success = true;
        response.Message = _T("Subscribed");

        // Emit subscriptionchanged event outside of lock content but still safe as strings copied
        event_subscriptionchanged(Core::JSON::String(sub.appId), Core::JSON::String(_T("subscribed")));

        return Core::ERROR_NONE;
    }

    uint32_t AppNotifications::endpoint_unsubscribe(const Core::JSON::String& appId, Types::StatusResponse& response)
    {
        if (!appId.IsSet() || appId.Value().empty()) {
            response.Success = false;
            response.Message = _T("Missing appId");
            return Core::ERROR_BAD_REQUEST;
        }

        bool removed = false;
        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
            auto it = _subscriptions.find(appId.Value());
            if (it != _subscriptions.end()) {
                _subscriptions.erase(it);
                removed = true;
            }
        }

        if (removed) {
            response.Success = true;
            response.Message = _T("Unsubscribed");
            event_subscriptionchanged(Core::JSON::String(appId.Value()), Core::JSON::String(_T("unsubscribed")));
            return Core::ERROR_NONE;
        } else {
            response.Success = false;
            response.Message = _T("Not subscribed");
            return Core::ERROR_UNKNOWN_KEY;
        }
    }

    bool AppNotifications::IsSubscribedUnlocked(const string& appId, const string& type) const
    {
        auto it = _subscriptions.find(appId);
        if (it == _subscriptions.end()) {
            return false;
        }
        const Subscription& sub = it->second;
        if (sub.types.empty()) {
            return true; // wildcard
        }
        return (sub.types.find(type) != sub.types.end());
    }

    uint32_t AppNotifications::endpoint_publish(const Types::NotificationMessage& message, Types::StatusResponse& response)
    {
        // Basic validation
        if (!message.AppId.IsSet() || message.AppId.Value().empty()) {
            response.Success = false;
            response.Message = _T("Missing appId");
            return Core::ERROR_BAD_REQUEST;
        }
        const string appId = message.AppId.Value();
        const string type = message.Type.IsSet() ? message.Type.Value() : string();

        // Subscription policy
        {
            Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);

            if (!_config.AllowPublishWithoutSubscription.Value() && !IsSubscribedUnlocked(appId, type)) {
                response.Success = false;
                response.Message = _T("App not subscribed for this type");
                return Core::ERROR_PRIVILEGED_REQUEST;
            }

            // Retain (bounded)
            if (_config.MaxQueue.Value() > 0) {
                _retained.push_back(message);
                while (_retained.size() > _config.MaxQueue.Value()) {
                    _retained.pop_front();
                }
            }
        }

        // Broadcast to clients: event_notification
        event_notification(message);

        response.Success = true;
        response.Message = _T("Published");
        return Core::ERROR_NONE;
    }

    uint32_t AppNotifications::endpoint_listSubscriptions(Core::JSON::ArrayType<Core::JSON::Object>& response)
    {
        Core::SafeSyncType<Core::CriticalSection> scope(_adminLock);
        for (const auto& kv : _subscriptions) {
            const Subscription& s = kv.second;

            Core::JSON::Object entry;
            entry.Set(_T("appId"), Core::JSON::String(s.appId.c_str()));
            Core::JSON::ArrayType<Core::JSON::String> types;
            for (const auto& t : s.types) {
                Core::JSON::String ts;
                ts = t.c_str();
                types.Add(ts);
            }
            entry.Set(_T("types"), types);

            response.Add(entry);
        }
        return Core::ERROR_NONE;
    }

    void AppNotifications::event_notification(const Types::NotificationMessage& message)
    {
        Notify(_T("notification"), message);
    }

    void AppNotifications::event_subscriptionchanged(const Core::JSON::String& appId, const Core::JSON::String& action)
    {
        Core::JSON::Object payload;
        payload.Set(_T("appId"), appId);
        payload.Set(_T("action"), action);
        Notify(_T("subscriptionchanged"), payload);
    }

} // namespace Plugin
} // namespace WPEFramework
