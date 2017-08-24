/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_connector_unixdomainsocket.cpp
 * @date    6/8/2015
 * @author  Peter Spiess-Knafl <dev@spiessknafl.at>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifdef UNIXDOMAINSOCKET_TESTING
#include "mockclientconnectionhandler.h"
#include <catch.hpp>
#include <jsonrpccpp/client/connectors/unixdomainsocketclient.h>
#include <jsonrpccpp/server/connectors/unixdomainsocketserver.h>

#include "checkexception.h"

using namespace jsonrpc;
using namespace std;

#define TEST_MODULE "[connector_unixdomainsocket]"

namespace testunixdomainsocketserver {
struct F {
  string filename;
  UnixDomainSocketServer server;
  UnixDomainSocketClient client;
  MockClientConnectionHandler handler;

  F() : filename(tmpnam(nullptr)), server(filename), client(filename) {
    server.SetHandler(&handler);
    REQUIRE(server.StartListening());
  }
  ~F() {
    server.StopListening();
    remove(filename.c_str());
  }
};

bool check_exception1(JsonRpcException const &ex) {
  return ex.GetCode() == Errors::ERROR_CLIENT_CONNECTOR;
}
}
using namespace testunixdomainsocketserver;

TEST_CASE_METHOD(F, "test_unixdomainsocket_success", TEST_MODULE) {
  handler.response = "exampleresponse";
  handler.timeout = 100;
  string result;
  string request = "examplerequest";
  string expectedResult = "exampleresponse";

  client.SendRPCMessage(request, result);

  CHECK(handler.request == request);
  CHECK(result == expectedResult);
}

TEST_CASE("test_unixdomainsocket_server_multiplestart", TEST_MODULE) {
  string filename = tmpnam(nullptr);

  UnixDomainSocketServer server(filename);
  CHECK(server.StartListening() == true);
  CHECK(server.StartListening() == false);

  UnixDomainSocketServer server2(filename);
  CHECK(server2.StartListening() == false);
  CHECK(server2.StopListening() == false);

  CHECK(server.StopListening() == true);
}

TEST_CASE("test_unixdomainsocket_client_invalid", TEST_MODULE) {
  UnixDomainSocketClient client("tmp/someinvalidpath");
  string result;
  CHECK_EXCEPTION_TYPE(client.SendRPCMessage("foobar", result),
                       JsonRpcException, check_exception1);
}

#endif
