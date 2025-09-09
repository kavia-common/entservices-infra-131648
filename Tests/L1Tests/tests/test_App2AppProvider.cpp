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

    void EnableProvider() {
        string resp;
        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("configure"),
            _T("{\"enabled\":true}"), resp));
        EXPECT_THAT(resp, ::testing::HasSubstr("\"enabled\""));
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
    EnableProvider();

    // Register AppA
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("registerApp"),
        _T("{\"appId\":\"AppA\"}"), response));
    EXPECT_THAT(response, ::testing::HasSubstr("\"registered\":true"));

    // Register duplicate AppA
    string resp2;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("registerApp"),
        _T("{\"appId\":\"AppA\"}"), resp2));
    // Implementation returns false for duplicate (since set insert fails)
    EXPECT_THAT(resp2, ::testing::HasSubstr("\"registered\":false"));

    // List apps should contain AppA
    string listResp;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("listApps"), _T("{}"), listResp));
    EXPECT_THAT(listResp, ::testing::HasSubstr("AppA"));
}

// A2P-6/A2P-7: unregister app and unknown
TEST_F(App2AppProviderTest, UnregisterKnownAndUnknown) {
    EnableProvider();

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
    EnableProvider();

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

    EnableProvider();

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

// A2P-11: Disabled gating - ensure methods are unavailable when disabled
TEST_F(App2AppProviderTest, DisabledGatingEnforced) {
    // By default, provider starts Disabled (no enabled=true in config)

    // registerApp
    string regResp;
    EXPECT_EQ(Core::ERROR_UNAVAILABLE, handler.Invoke(connection, _T("registerApp"),
        _T("{\"appId\":\"AppA\"}"), regResp));
    EXPECT_THAT(regResp, ::testing::HasSubstr("Service unavailable"));

    // unregisterApp
    string unregResp;
    EXPECT_EQ(Core::ERROR_UNAVAILABLE, handler.Invoke(connection, _T("unregisterApp"),
        _T("{\"appId\":\"AppA\"}"), unregResp));
    EXPECT_THAT(unregResp, ::testing::HasSubstr("Service unavailable"));

    // sendMessage
    string sendResp;
    EXPECT_EQ(Core::ERROR_UNAVAILABLE, handler.Invoke(connection, _T("sendMessage"),
        _T("{\"from\":\"A\",\"to\":\"B\",\"payload\":\"x\"}"), sendResp));
    EXPECT_THAT(sendResp, ::testing::HasSubstr("Service unavailable"));

    // listApps
    string listResp;
    EXPECT_EQ(Core::ERROR_UNAVAILABLE, handler.Invoke(connection, _T("listApps"), _T("{}"), listResp));
    EXPECT_THAT(listResp, ::testing::HasSubstr("Service unavailable"));

    // ping should work
    string pingResp;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("ping"), _T("\"\""), pingResp));
    EXPECT_EQ(string("\"pong\""), pingResp);

    // getInfo should work
    string infoResp;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInfo"), _T("{}"), infoResp));
    EXPECT_THAT(infoResp, ::testing::HasSubstr("state"));

    // configure should work (e.g. set providerId only, without enabling)
    string cfgResp;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("configure"),
        _T("{\"providerId\":\"prov-disabled\"}"), cfgResp));
    EXPECT_THAT(cfgResp, ::testing::HasSubstr("prov-disabled"));
}

// A2P-12: Parameter validation errors and messages for registerApp, unregisterApp, sendMessage, and configure
TEST_F(App2AppProviderTest, ParameterValidationErrors) {
    EnableProvider();

    // registerApp: missing appId
    string resp;
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("registerApp"), _T("{}"), resp));
    EXPECT_THAT(resp, ::testing::HasSubstr("Validation error"));
    EXPECT_THAT(resp, ::testing::HasSubstr("appId"));
    EXPECT_THAT(resp, ::testing::HasSubstr("required"));

    // registerApp: empty appId
    resp.clear();
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("registerApp"),
        _T("{\"appId\":\"\"}"), resp));
    EXPECT_THAT(resp, ::testing::HasSubstr("Validation error"));
    EXPECT_THAT(resp, ::testing::HasSubstr("non-empty"));

    // unregisterApp: missing appId
    resp.clear();
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("unregisterApp"), _T("{}"), resp));
    EXPECT_THAT(resp, ::testing::HasSubstr("Validation error"));
    EXPECT_THAT(resp, ::testing::HasSubstr("appId"));
    EXPECT_THAT(resp, ::testing::HasSubstr("required"));

    // unregisterApp: empty appId
    resp.clear();
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("unregisterApp"),
        _T("{\"appId\":\"\"}"), resp));
    EXPECT_THAT(resp, ::testing::HasSubstr("Validation error"));
    EXPECT_THAT(resp, ::testing::HasSubstr("non-empty"));

    // sendMessage: missing fields
    resp.clear();
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("sendMessage"),
        _T("{\"from\":\"A\"}"), resp));
    EXPECT_THAT(resp, ::testing::HasSubstr("Validation error"));
    EXPECT_THAT(resp, ::testing::HasSubstr("'from', 'to', and 'payload' are required"));

    // sendMessage: empty from and to
    resp.clear();
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("sendMessage"),
        _T("{\"from\":\"\",\"to\":\"\",\"payload\":\"x\"}"), resp));
    EXPECT_THAT(resp, ::testing::HasSubstr("Validation error"));
    EXPECT_THAT(resp, ::testing::HasSubstr("'from' must be a non-empty string"));

    // configure: invalid providerId (empty)
    resp.clear();
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("configure"),
        _T("{\"providerId\":\"\"}"), resp));
    EXPECT_THAT(resp, ::testing::HasSubstr("Validation error"));
    EXPECT_THAT(resp, ::testing::HasSubstr("providerId"));
    EXPECT_THAT(resp, ::testing::HasSubstr("non-empty"));

    // configure: invalid capabilities (contains null)
    resp.clear();
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("configure"),
        _T("{\"capabilities\":[null]}"), resp));
    EXPECT_THAT(resp, ::testing::HasSubstr("Validation error"));
    EXPECT_THAT(resp, ::testing::HasSubstr("capabilities"));
}

// Negative cases: missing fields (legacy test updated to match current error codes/messages)
TEST_F(App2AppProviderTest, NegativeMissingFields) {
    EnableProvider();

    // Missing appId in register
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("registerApp"), _T("{}"), response));
    EXPECT_THAT(response, ::testing::HasSubstr("Validation error"));

    // Missing appId in unregister
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("unregisterApp"), _T("{}"), response));
    EXPECT_THAT(response, ::testing::HasSubstr("Validation error"));

    // Missing message fields
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("sendMessage"), _T("{\"from\":\"A\"}"), response));
    EXPECT_THAT(response, ::testing::HasSubstr("Validation error"));
}
