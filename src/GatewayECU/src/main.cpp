#include "GatewayDbusService.h"
#include "EcuSomeIpClient.h"
#include <sdbus-c++/sdbus-c++.h>
#include <iostream>
#include <csignal>
#include <atomic>

static std::atomic<bool> running{true};

int main()
{
    std::signal(SIGINT,  [](int){ running = false; });
    std::signal(SIGTERM, [](int){ running = false; });

    try
    {
        // SOME/IP client to ECU Simulator
        auto someipClient = std::make_shared<EcuSomeIpClient>("gateway-ecu");

        // D-Bus system bus, request well-known service name
        auto connection = sdbus::createSystemBusConnection(
            sdbus::ServiceName{"com.vehicle.gateway"});

        GatewayDbusService service(*connection, someipClient);

        std::cout << "[GatewayECU] D-Bus service registered, entering event loop\n";

        // Run event loop until SIGTERM / SIGINT
        connection->enterEventLoopAsync();
        while (running) std::this_thread::sleep_for(std::chrono::milliseconds(200));

        std::cout << "[GatewayECU] Shutting down\n";
    }
    catch (const std::exception& ex)
    {
        std::cerr << "[GatewayECU] Fatal: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
