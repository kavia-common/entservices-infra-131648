#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "AppGateway.h"
#include "ServiceMock.h"
#include <core/core.h>
#include "ThunderPortability.h"

using namespace WPEFramework;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;
using std::string;

// Fixture for AppGateway tests
class AppGatewayTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::AppGateway> plugin;
    Core::JSONRPC::Handler& handler;
    DECL_CORE_JSONRPC_CONX connection;
    string response;

    NiceMock<ServiceMock> service;

    AppGatewayTest()
        : plugin(Core::ProxyType<Plugin::AppGateway>::Create())
        , handler(*plugin)
        , INIT_CONX(1, 0) {
    }

    void SetUp() override {
        // Default mock behavior: no special interfaces
        ON_CALL(service, QueryInterfaceByCallsign(_, _)).WillByDefault(Return(nullptr));
        EXPECT_EQ(string(), plugin->Initialize(&service));
        // Endpoints are registered in constructor, but ensure registration path testability
        plugin->RegisterAll();
    }

    void TearDown() override {
        plugin->UnregisterAll();
        plugin->Deinitialize(&service);
    }
};

// AG-1: ping returns pong
TEST_F(AppGatewayTest, PingReturnsPong) {
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("ping"), _T("\"\""), response));
    EXPECT_EQ(string("\"pong\""), response);
}

// AG-2: getInfo default after initialize
TEST_F(AppGatewayTest, GetInfoDefault) {
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInfo"), _T("{}"), response));
    // Response should be an object with name, version, state
    EXPECT_THAT(response, ::testing::HasSubstr("name"));
    EXPECT_THAT(response, ::testing::HasSubstr("version"));
    EXPECT_THAT(response, ::testing::HasSubstr("state"));
}

// AG-3: setConfig enable path
TEST_F(AppGatewayTest, SetConfigEnableUpdatesState) {
    // Provide enabled true and routeTable string
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setConfig"),
        _T("{\"enabled\":true,\"routeTable\":\"/etc/gateway/gateway.config.json\"}"), response));

    // Verify echo back fields exist
    EXPECT_THAT(response, ::testing::HasSubstr("\"enabled\""));
    EXPECT_THAT(response, ::testing::HasSubstr("\"routeTable\""));

    // getInfo reflects updated state
    string info;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInfo"), _T("{}"), info));
    EXPECT_THAT(info, ::testing::HasSubstr("Enabled"));
}

// AG-4: setConfig invalid types are still parsed as JSON::Object; our minimal impl sets values if present; here we simulate wrong types and expect no crash and error_none
TEST_F(AppGatewayTest, SetConfigInvalidTypesDoesNotCrash) {
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setConfig"),
        _T("{\"enabled\":\"yes\",\"routeTable\":123}"), response));

    // Since current plugin accepts any type assignment through JSON container, just ensure a consistent JSON response
    EXPECT_THAT(response, ::testing::HasSubstr("\"enabled\""));
    EXPECT_THAT(response, ::testing::HasSubstr("\"routeTable\""));
}

// AG-6: Deinitialize/Initialize idempotency-like path
TEST_F(AppGatewayTest, ReinitializeLifecycle) {
    plugin->UnregisterAll();
    plugin->Deinitialize(&service);
    EXPECT_EQ(string(), plugin->Initialize(&service));
    plugin->RegisterAll();

    // Invoke ping to ensure all works after reinit
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("ping"), _T("\"\""), response));
    EXPECT_EQ(string("\"pong\""), response);
}
