#pragma once

namespace FleetTelemetry
{
    class ServerListener
    {
    public:
        bool Start(int port);
        void Stop();
    private:
        bool m_running = false;
    };
}
