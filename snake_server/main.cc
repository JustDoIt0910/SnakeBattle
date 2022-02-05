#include "server/server.h"

int main(int argc, char* argv[])
{
    Server* server = new Server(9000, 9001);
    server->Serve();
    return 0; 
}