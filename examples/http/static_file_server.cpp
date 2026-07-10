#include "agora/net/http/static_file_server.h"
#include "agora/net/event_loop.h"
#include "agora/base/logger.h"

using namespace agora::net;
using namespace agora::net::http;

int main() {
    EventLoop loop;
    
    StaticFileServer server(
        &loop,
        InetAddress(8000),
        "StaticFileServer",
        "/home/petrichor/AgoraIM/www"  // 绝对路径
    );
    
    server.setThreadNum(4);
    server.start();
    
    LOG_INFO("StaticFileServer listening on http://localhost:8000");
    loop.loop();
    
    return 0;
}