/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_integration.cpp
 * @date    28.09.2013
 * @author  Peter Spiess-Knafl <dev@spiessknafl.at>
 * @license See attached LICENSE.txt
 ************************************************************************/


#ifdef STUBGEN_TESTING
#include <catch.hpp>

#ifdef HTTP_TESTING
#include <jsonrpccpp/server/connectors/httpserver.h>
#include <jsonrpccpp/client/connectors/httpclient.h>
#endif

#include "gen/abstractstubserver.h"
#include "gen/stubclient.h"

using namespace jsonrpc;
using namespace std;

#define TEST_PORT 8383
#define CLIENT_URL "http://localhost:8383"

#define TEST_MODULE "[integration]"

class StubServer : public AbstractStubServer {
    public:
        StubServer(AbstractServerConnector &connector) : AbstractStubServer(connector) {}
        virtual void notifyServer() {

        }

        virtual std::string sayHello(const std::string& name)
        {
            return string("Hello ") + name;
        }

        virtual int addNumbers(int param1, int param2)
        {
            return param1+param2;
        }

        virtual double addNumbers2(double param1, double param2)
        {
            return param1 + param2;
        }

        virtual bool isEqual(const std::string& str1, const std::string &str2)
        {
            return str1 == str2;
        }

        virtual Json::Value buildObject(const std::string &name, int age)
        {
            Json::Value result;
            result["name"] = name;
            result["age"] = age;
            return result;
        }

        virtual std::string methodWithoutParameters()
        {
            return "foo";
        }
};

#ifdef HTTP_TESTING

TEST_CASE("test_integration_http", TEST_MODULE)
{
    HttpServer sconn(TEST_PORT);
    HttpClient cconn(CLIENT_URL);
    StubServer server(sconn);
    server.StartListening();
    StubClient client(cconn);

    CHECK(client.addNumbers(3,4) == 7);
    CHECK(client.addNumbers2(3.2,4.2) == 7.4);
    CHECK(client.sayHello("Test") == "Hello Test");
    CHECK(client.methodWithoutParameters() == "foo");
    CHECK(client.isEqual("str1", "str1") == true);
    CHECK(client.isEqual("str1", "str2") == false);

    Json::Value result = client.buildObject("Test", 33);
    CHECK(result["name"].asString() == "Test");
    CHECK(result["age"].asInt() == 33);

    server.StopListening();
}

#endif
#ifdef UNIXDOMAINSOCKET_TESTING

#include <jsonrpccpp/server/connectors/unixdomainsocketserver.h>
#include <jsonrpccpp/client/connectors/unixdomainsocketclient.h>
/*
TEST_CASE("test_integration_unixdomain", TEST_MODULE)
{
    string filename = "/tmp/jsonrpcunixdomain";
    UnixDomainSocketServer sconn(filename);
    UnixDomainSocketClient cconn(filename);

    StubServer server(sconn);
    server.StartListening();
    StubClient client(cconn);

    CHECK(client.addNumbers(3,4) == 7);
    CHECK(client.addNumbers2(3.2,4.2) == 7.4);
    CHECK(client.sayHello("Test") == "Hello Test");
    CHECK(client.methodWithoutParameters() == "foo");
    CHECK(client.isEqual("str1", "str1") == true);
    CHECK(client.isEqual("str1", "str2") == false);

    Json::Value result = client.buildObject("Test", 33);
    CHECK(result["name"].asString() == "Test");
    CHECK(result["age"].asInt() == 33);

    server.StopListening();
    remove(filename.c_str());
}*/
#endif
#ifdef FILEDESCRIPTOR_TESTING

#include <jsonrpccpp/server/connectors/filedescriptorserver.h>
#include <jsonrpccpp/client/connectors/filedescriptorclient.h>


TEST_CASE("test_integration_filedescriptor", TEST_MODULE)
{
    int c2sfd[2]; // Client to server fd
    int s2cfd[2]; // Server to client fd
    pipe(c2sfd);
    pipe(s2cfd);

    FileDescriptorServer sconn(c2sfd[0], s2cfd[1]);
    FileDescriptorClient cconn(s2cfd[0], c2sfd[1]);

    StubServer server(sconn);
    server.StartListening();
    StubClient client(cconn);

    CHECK(client.addNumbers(3,4) == 7);
    CHECK(client.addNumbers2(3.2,4.2) == 7.4);
    CHECK(client.sayHello("Test") == "Hello Test");
    CHECK(client.methodWithoutParameters() == "foo");
    CHECK(client.isEqual("str1", "str1") == true);
    CHECK(client.isEqual("str1", "str2") == false);

    Json::Value result = client.buildObject("Test", 33);
    CHECK(result["name"].asString() == "Test");
    CHECK(result["age"].asInt() == 33);

    server.StopListening();

    close(c2sfd[0]);
    close(c2sfd[1]);
    close(s2cfd[0]);
    close(s2cfd[1]);
}
#endif


#endif
