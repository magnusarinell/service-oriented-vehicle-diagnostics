#include "DiagService.cpp"
#include <iostream>
#include <csignal>

int main()
{
    DiagService svc;
    if (!svc.init()) {
        std::cerr << "[EcuSimulator] Initialisation failed\n";
        return 1;
    }
    std::cout << "[EcuSimulator] Starting SOME/IP service…\n";
    svc.start();   // blocks until application stopped
    return 0;
}
