#pragma once

#include "Module.h"
#include "AppNotificationsTypes.h"

#include <core/JSON.h>
#include <plugins/JSONRPC.h>
#include <plugins/Plugin.h>

namespace WPEFramework {
namespace Plugin {

/**
 * PUBLIC_INTERFACE
 * AppNotifications Thunder plugin brokers application notifications between
 * apps, providers, and clients via JSON-RPC, following the App Gateway design.
 *
 * JSON-RPC interface: "org.rdk.AppNotifications.1"
 * Methods:
 *  - ping()
 *  - getInfo()
 *  - subscribe(params: SubscriptionRequest)
 *  - unsubscribe(params: { appId })
 *  - publish(message: NotificationMessage)
 *  - listSubscriptions() -> [ { appId, types[] } ]
 * Events:
 *  - notification: NotificationMessage
 *  - subscriptionchanged: { appId, action: "subscribed"|"unsubscribed" }
 */
class AppNotifications : public PluginHost::IPlugin, public PluginHost::JSONRPC {
public:
    // PUBLIC_INTERFACE
    class Config : public Core::JSON::Container {
    public:
        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;

        Config()
            : Core::JSON::Container()
        {
            Add(_T("enabled"), &Enabled);
            Add(_T("maxQueue"), &MaxQueue);
            Add(_T("allowPublishWithoutSubscription"), &AllowPublishWithoutSubscription);
        }

        Core::JSON::Boolean Enabled;
        Core::JSON::DecUInt16 MaxQueue; // backlog for retained notifications if needed
        Core::JSON::Boolean AllowPublishWithoutSubscription;
    };

public:
    AppNotifications(const AppNotifications&) = delete;
    AppNotifications& operator=(const AppNotifications&) = delete;

    // PUBLIC_INTERFACE
    AppNotifications();
    ~AppNotifications() override;

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

    static constexpr const TCHAR* ServiceName() { return _T("org.rdk.AppNotifications"); }
    static constexpr const uint8_t Version = 1;

private:
    // JSON-RPC endpoint handlers
    uint32_t endpoint_ping(Core::JSON::String& response);
    uint32_t endpoint_getInfo(Core::JSON::Object& response);
    uint32_t endpoint_subscribe(const Types::SubscriptionRequest& request, Types::StatusResponse& response);
    uint32_t endpoint_unsubscribe(const Core::JSON::String& appId, Types::StatusResponse& response);
    uint32_t endpoint_publish(const Types::NotificationMessage& message, Types::StatusResponse& response);
    uint32_t endpoint_listSubscriptions(Core::JSON::ArrayType<Core::JSON::Object>& response);

    // Event helpers
    void event_notification(const Types::NotificationMessage& message);
    void event_subscriptionchanged(const Core::JSON::String& appId, const Core::JSON::String& action);

    // Internal helpers
    bool IsSubscribedUnlocked(const string& appId, const string& type) const;

private:
    PluginHost::IShell* _service;
    Config _config;
    mutable Core::CriticalSection _adminLock;

    struct Subscription {
        string appId;
        std::set<string> types; // empty means all
    };

    // By appId
    std::map<string, Subscription> _subscriptions;

    // Retained queue (by simple vector, bounded by MaxQueue)
    std::deque<Types::NotificationMessage> _retained;
};

} // namespace Plugin
} // namespace WPEFramework
