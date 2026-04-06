#include "ServerApp.h"

int main(int argc, char* argv[])
{
    FleetTelemetry::ServerApp app;
    return app.Run(argc, argv);
}
