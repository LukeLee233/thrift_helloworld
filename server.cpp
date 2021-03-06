#include <thrift/concurrency/ThreadManager.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/TToString.h>

#include <iostream>
#include <stdexcept>
#include <sstream>

#include "gen-cpp/Calculator.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace helloworld;
using namespace common;

class CalculatorHandler : public CalculatorIf {
public:
    CalculatorHandler() {}

    void ping() { cout << "ping()" << endl; }

    int32_t add(const int32_t n1, const int32_t n2) {
        cout << "add(" << n1 << ", " << n2 << ")" << endl;
        return n1 + n2;
    }

    int32_t calculate(const int32_t logid, const Work& work) {
        cout << "calculate(" << logid << ", " << work << ")" << endl;
        int32_t val;

        switch (work.op) {
            case Operation::ADD:
                val = work.num1 + work.num2;
                break;
            case Operation::SUBTRACT:
                val = work.num1 - work.num2;
                break;
            case Operation::MULTIPLY:
                val = work.num1 * work.num2;
                break;
            case Operation::DIVIDE:
                if (work.num2 == 0) {
                    InvalidOperation io;
                    io.whatOp = work.op;
                    io.why = "Cannot divide by 0";
                    throw io;
                }
                val = work.num1 / work.num2;
                break;
            default:
                InvalidOperation io;
                io.whatOp = work.op;
                io.why = "Invalid Operation";
                throw io;
        }

        SharedStruct ss;
        ss.key = logid;
        ss.value = to_string(val);

        log[logid] = ss;

        return val;
    }

    void getStruct(SharedStruct& ret, const int32_t logid) {
        cout << "getStruct(" << logid << ")" << endl;
        ret = log[logid];
    }

    void zip() { cout << "zip()" << endl; }

protected:
    map<int32_t, SharedStruct> log;
};

/*
  CalculatorIfFactory is code generated.
  CalculatorCloneFactory is useful for getting access to the server side of the
  transport.  It is also useful for making per-connection state.  Without this
  CloneFactory, all connections will end up sharing the same handler instance.
*/
class CalculatorCloneFactory : virtual public CalculatorIfFactory {
public:
    virtual ~CalculatorCloneFactory() {}
    virtual CalculatorIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo)
    {
        ::std::shared_ptr<TSocket> sock = ::std::dynamic_pointer_cast<TSocket>(connInfo.transport);
        cout << "Incoming connection\n";
        cout << "\tSocketInfo: "  << sock->getSocketInfo() << "\n";
        cout << "\tPeerHost: "    << sock->getPeerHost() << "\n";
        cout << "\tPeerAddress: " << sock->getPeerAddress() << "\n";
        cout << "\tPeerPort: "    << sock->getPeerPort() << "\n";
        return new CalculatorHandler;
    }
    virtual void releaseHandler( ::common::SharedServiceIf* handler) {
        delete handler;
    }
};

int main() {
    TThreadedServer server(
        ::std::make_shared<CalculatorProcessorFactory>(::std::make_shared<CalculatorCloneFactory>()),
        ::std::make_shared<TServerSocket>(9090), //port
        ::std::make_shared<TBufferedTransportFactory>(),
        ::std::make_shared<TBinaryProtocolFactory>());

    /*
    // if you don't need per-connection state, do the following instead
    TThreadedServer server(
      stdcxx::make_shared<CalculatorProcessor>(stdcxx::make_shared<CalculatorHandler>()),
      stdcxx::make_shared<TServerSocket>(9090), //port
      stdcxx::make_shared<TBufferedTransportFactory>(),
      stdcxx::make_shared<TBinaryProtocolFactory>());
    */

    /**
     * Here are some alternate server types...

    // This server only allows one connection at a time, but spawns no threads
    TSimpleServer server(
      stdcxx::make_shared<CalculatorProcessor>(stdcxx::make_shared<CalculatorHandler>()),
      stdcxx::make_shared<TServerSocket>(9090),
      stdcxx::make_shared<TBufferedTransportFactory>(),
      stdcxx::make_shared<TBinaryProtocolFactory>());

    const int workerCount = 4;

    stdcxx::shared_ptr<ThreadManager> threadManager =
      ThreadManager::newSimpleThreadManager(workerCount);
    threadManager->threadFactory(
      stdcxx::make_shared<PlatformThreadFactory>());
    threadManager->start();

    // This server allows "workerCount" connection at a time, and reuses threads
    TThreadPoolServer server(
      stdcxx::make_shared<CalculatorProcessorFactory>(stdcxx::make_shared<CalculatorCloneFactory>()),
      stdcxx::make_shared<TServerSocket>(9090),
      stdcxx::make_shared<TBufferedTransportFactory>(),
      stdcxx::make_shared<TBinaryProtocolFactory>(),
      threadManager);
    */

    cout << "Starting the server..." << endl;
    server.serve();
    cout << "Done." << endl;
    return 0;
}