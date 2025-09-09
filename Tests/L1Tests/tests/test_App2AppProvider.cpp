#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "App2AppProvider.h"
#include "ServiceMock.h"
#include <core/core.h>
#include "ThunderPortability.h"

using namespace WPEFramework;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;
using std::string;

// Fixture for App2AppProvider tests
class App2AppProviderTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::App2AppProvider> plugin;
    Core::JSONRPC::Handler& handler;
    DECL_CORE_JSONRPC_CONX connection;
    string response;

    NiceMock<ServiceMock> service;

    App2AppProviderTest()
        : plugin(Core::ProxyType<Plugin::App2AppProvider>::Create())
        , handler(*plugin)
        , INIT_CONX(1, 0) {}

    void SetUp() override {
        ON_CALL(service, QueryInterfaceByCallsign(_, _)).WillByDefault(Return(nullptr));
        EXPECT_EQ(string(), plugin->Initialize(&service));
        plugin->RegisterAll();
    }

    void TearDown() override {
        plugin->UnregisterAll();
        plugin->Deinitialize(&service);
    }
};

// A2P-1: ping
TEST_F(App2AppProviderTest, PingReturnsPong) {
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("ping"), _T("\"\""), response));
    EXPECT_EQ(string("\"pong\""), response);
}

// A2P-2: getInfo default
TEST_F(App2AppProviderTest, GetInfoDefault) {
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInfo"), _T("{}"), response));
    EXPECT_THAT(response, ::testing::HasSubstr("name"));
    EXPECT_THAT(response, ::testing::HasSubstr("version"));
    EXPECT_THAT(response, ::testing::HasSubstr("state"));
    EXPECT_THAT(response, ::testing::HasSubstr("capabilities"));
}

// A2P-3: configure provider with capabilities and id
TEST_F(App2AppProviderTest, ConfigureSetsStateAndFields) {
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("configure"),
        _T("{\"enabled\":true,\"providerId\":\"prov-1\",\"capabilities\":[\"IntegratedPlayer.create\"]}"), response));
    EXPECT_THAT(response, ::testing::HasSubstr("\"enabled\""));
    EXPECT_THAT(response, ::testing::HasSubstr("\"providerId\""));
    EXPECT_THAT(response, ::testing::HasSubstr("\"capabilities\""));

    string info;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInfo"), _T("{}"), info));
    EXPECT_THAT(info, ::testing::HasSubstr("prov-1"));
    EXPECT_THAT(info, ::testing::HasSubstr("IntegratedPlayer.create"));
}

// A2P-4/A2P-5: register app and idempotency
TEST_F(App2AppProviderTest, RegisterAppAndDuplicate) {
    // Register AppA
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("registerApp"),
        _T("{\"appId\":\"AppA\"}"), response));
    EXPECT_THAT(response, ::testing::HasSubstr("\"registered\":true"));

    // Register duplicate AppA
    string resp2;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("registerApp"),
        _T("{\"appId\":\"AppA\"}"), resp2));
    // Implementation returns false for duplicate (since set insert fails)
    // Accept either true (idempotent) or false depending on impl; here expect false
    EXPECT_THAT(resp2, ::testing::HasSubstr("\"registered\":false"));

    // List apps should contain AppA
    string listResp;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("listApps"), _T("{}"), listResp));
    EXPECT_THAT(listResp, ::testing::HasSubstr("AppA"));
}

// A2P-6/A2P-7: unregister app and unknown
TEST_F(App2AppProviderTest, UnregisterKnownAndUnknown) {
    // Pre: register AppA
    ASSERT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("registerApp"),
        _T("{\"appId\":\"AppA\"}"), response));

    // Unregister AppA
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("unregisterApp"),
        _T("{\"appId\":\"AppA\"}"), response));
    EXPECT_THAT(response, ::testing::HasSubstr("\"unregistered\":true"));

    // Unregister again should return false
    string resp2;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("unregisterApp"),
        _T("{\"appId\":\"AppA\"}"), resp2));
    EXPECT_THAT(resp2, ::testing::HasSubstr("\"unregistered\":false"));
}

// A2P-8/A2P-9: send message success and fail
TEST_F(App2AppProviderTest, SendMessageSuccessAndFail) {
    // Register A and B
    ASSERT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("registerApp"),
        _T("{\"appId\":\"AppA\"}"), response));
    ASSERT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("registerApp"),
        _T("{\"appId\":\"AppB\"}"), response));

    // Success: to registered recipient
    string sendOk;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendMessage"),
        _T("{\"from\":\"AppA\",\"to\":\"AppB\",\"payload\":\"{\\\"key\\\":\\\"value\\\"}\"}"), sendOk));
    EXPECT_THAT(sendOk, ::testing::HasSubstr("\"delivered\":true"));

    // Fail: unknown recipient
    string sendFail;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendMessage"),
        _T("{\"from\":\"AppA\",\"to\":\"Unknown\",\"payload\":\"{}\"}"), sendFail));
    EXPECT_THAT(sendFail, ::testing::HasSubstr("\"delivered\":false"));
}

// A2P-10: list apps after multiple operations
TEST_F(App2AppProviderTest, ListAppsAfterOps) {
    // Clean slate by reinit
    plugin->UnregisterAll();
    plugin->Deinitialize(&service);
    EXPECT_EQ(string(), plugin->Initialize(&service));
    plugin->RegisterAll();

    ASSERT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("registerApp"),
        _T("{\"appId\":\"AppA\"}"), response));
    ASSERT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("registerApp"),
        _T("{\"appId\":\"AppB\"}"), response));
    ASSERT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("unregisterApp"),
        _T("{\"appId\":\"AppA\"}"), response));

    string listResp;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("listApps"), _T("{}"), listResp));
    EXPECT_THAT(listResp, ::testing::HasSubstr("AppB"));
    EXPECT_THAT(listResp, ::testing::Not(::testing::HasSubstr("AppA")));
}

// Negative cases: missing fields
TEST_F(App2AppProviderTest, NegativeMissingFields) {
    // Missing appId in register
    EXPECT_EQ(Core::ERROR_BAD_REQUEST, handler.Invoke(connection, _T("registerApp"), _T("{}"), response));

    // Missing appId in unregister
    EXPECT_EQ(Core::ERROR_BAD_REQUEST, handler.Invoke(connection, _T("unregisterApp"), _T("{}"), response));

    // Missing message fields
    EXPECT_EQ(Core::ERROR_BAD_REQUEST, handler.Invoke(connection, _T("sendMessage"), _T("{\"from\":\"A\"}"), response));
}
