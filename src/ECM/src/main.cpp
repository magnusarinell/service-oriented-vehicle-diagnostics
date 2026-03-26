#include "DiagService.cpp"
#include <iostream>
#include <csignal>

int main()
{
    DiagService svc;
    if (!svc.init()) {
        std::cerr << "[ECM] Initialisation failed\n";
        return 1;
    }
    std::cout << "[ECM] Starting SOME/IP service…\n";
    svc.start();   // blocks until application stopped
    return 0;
}
