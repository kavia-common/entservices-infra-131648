#pragma once

#include "Module.h"
#include <core/JSON.h>

namespace WPEFramework {
namespace Plugin {
namespace Types {

    // PUBLIC_INTERFACE
    /**
     * Notification message payload representing application-originated notification.
     */
    class NotificationMessage : public Core::JSON::Container {
    public:
        NotificationMessage(const NotificationMessage&) = delete;
        NotificationMessage& operator=(const NotificationMessage&) = delete;

        NotificationMessage()
            : Core::JSON::Container()
        {
            Add(_T("appId"), &AppId);
            Add(_T("type"), &Type);
            Add(_T("title"), &Title);
            Add(_T("body"), &Body);
            Add(_T("timestampMs"), &TimestampMs);
            Add(_T("data"), &Data);
            Add(_T("priority"), &Priority);
        }

    public:
        Core::JSON::String AppId;
        Core::JSON::String Type;
        Core::JSON::String Title;
        Core::JSON::String Body;
        Core::JSON::DecUInt64 TimestampMs;
        Core::JSON::String Data;      // JSON string-ified metadata
        Core::JSON::DecUInt8 Priority; // 0..5
    };

    // PUBLIC_INTERFACE
    /**
     * Subscription request for app notifications by appId and optional types filter.
     */
    class SubscriptionRequest : public Core::JSON::Container {
    public:
        SubscriptionRequest(const SubscriptionRequest&) = delete;
        SubscriptionRequest& operator=(const SubscriptionRequest&) = delete;

        SubscriptionRequest()
            : Core::JSON::Container()
        {
            Add(_T("appId"), &AppId);
            Add(_T("types"), &Types);
        }

    public:
        Core::JSON::String AppId;
        Core::JSON::ArrayType<Core::JSON::String> Types;
    };

    // PUBLIC_INTERFACE
    /**
     * Generic status response with success flag and message.
     */
    class StatusResponse : public Core::JSON::Container {
    public:
        StatusResponse(const StatusResponse&) = delete;
        StatusResponse& operator=(const StatusResponse&) = delete;

        StatusResponse()
            : Core::JSON::Container()
        {
            Add(_T("success"), &Success);
            Add(_T("message"), &Message);
        }

    public:
        Core::JSON::Boolean Success;
        Core::JSON::String Message;
    };

} // namespace Types
} // namespace Plugin
} // namespace WPEFramework
