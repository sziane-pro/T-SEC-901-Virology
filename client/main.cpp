#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <iostream>
using namespace std;

ix::SocketTLSOptions makeClientTLSOptions(const string& dir)
{
    ix::SocketTLSOptions opts;
    opts.tls    = true;
    opts.caFile = dir + "/cert.pem";
    return opts;
}

int main(int argc, char** argv){

    ix::initNetSystem();

    auto tls = makeClientTLSOptions(argc > 1 ? argv[1] : "certs");
    if (!tls.isValid())
    {
        cerr << "TLS: " << tls.getErrorMsg() << endl;
        return 1;
    }

    ix::WebSocket webSocket;

    string url("wss://localhost:9090/");

    webSocket.setTLSOptions(tls);
    webSocket.setUrl(url);

    webSocket.setPingInterval(45);

    atomic<bool> connected{false};
    atomic<bool> done{false};

    webSocket.setOnMessageCallback([&connected, &done](const ix::WebSocketMessagePtr& msg){
        switch(msg->type){
            case ix::WebSocketMessageType::Open:
                cout << "Connected" << endl;
                connected = true;
                break;
            case ix::WebSocketMessageType::Message:
                if(msg->str == "quit"){
                    // signaler que le client doit s'arrêter
                    done = true;
                } else {
                    cout << "Received" << msg->str << endl;
                }
                break;
            case ix::WebSocketMessageType::Error:
                cerr << "Error" << msg->errorInfo.reason << endl;
                done=true;
                break;
            case ix::WebSocketMessageType::Close:
                cout << "Closed" << endl;
                connected=false;
                done=true;
                break;
            default:
                break;
        }
    });

    cout << "Connecting to: " << url << endl;
    webSocket.start();

    for(int i=0; i<50 && !connected; i++){
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    if(connected){
        webSocket.send("hello server");
    }

    while(connected && !done){
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    webSocket.stop();

    ix::uninitNetSystem();

    return 0;
}

