#include "GatewayDbusService.h"
#include <iostream>

static constexpr char DBUS_OBJECT_PATH[] = "/com/vehicle/gateway";

GatewayDbusService::GatewayDbusService(
    sdbus::IConnection& connection,
    std::shared_ptr<EcuSomeIpClient> someipClient)
    : AdaptorInterfaces(connection, sdbus::ObjectPath{DBUS_OBJECT_PATH})
    , someip_(std::move(someipClient))
{
    registerAdaptor();
}

GatewayDbusService::~GatewayDbusService()
{
    unregisterAdaptor();
}

std::string GatewayDbusService::GetCapabilities(const std::string& ecuId)
{
    std::cout << "[GW] → GetCapabilities(" << ecuId << ")\n";
    auto r = someip_->call(METHOD_GET_CAPABILITIES, ecuId);
    std::cout << "[GW] ← GetCapabilities " << r.size() << "B\n";
    return r;
}

std::string GatewayDbusService::ReadData(const std::string& ecuId)
{
    std::cout << "[GW] → ReadData(" << ecuId << ")\n";
    auto r = someip_->call(METHOD_READ_DATA, ecuId);
    std::cout << "[GW] ← ReadData " << r.size() << "B\n";
    return r;
}

std::string GatewayDbusService::ReadDataItem(const std::string& ecuIdAndDataId)
{
    std::cout << "[GW] → ReadDataItem(" << ecuIdAndDataId << ")\n";
    auto r = someip_->call(METHOD_READ_DATA_ITEM, ecuIdAndDataId);
    std::cout << "[GW] ← ReadDataItem " << r.size() << "B\n";
    return r;
}

std::string GatewayDbusService::GetFaults(const std::string& ecuId)
{
    std::cout << "[GW] → GetFaults(" << ecuId << ")\n";
    auto r = someip_->call(METHOD_GET_FAULTS, ecuId);
    std::cout << "[GW] ← GetFaults " << r.size() << "B\n";
    return r;
}

std::string GatewayDbusService::ClearFaults(const std::string& ecuId)
{
    std::cout << "[GW] → ClearFaults(" << ecuId << ")\n";
    someip_->call(METHOD_CLEAR_FAULTS, ecuId);
    std::cout << "[GW] ← ClearFaults ok\n";
    return R"({"status":"ok"})";
}

std::string GatewayDbusService::GetOperations(const std::string& ecuId)
{
    std::cout << "[GW] → GetOperations(" << ecuId << ")\n";
    auto r = someip_->call(METHOD_GET_OPERATIONS, ecuId);
    std::cout << "[GW] ← GetOperations " << r.size() << "B\n";
    return r;
}

std::string GatewayDbusService::ExecuteOperation(const std::string& ecuIdAndOpId)
{
    std::cout << "[GW] → ExecuteOperation(" << ecuIdAndOpId << ")\n";
    auto r = someip_->call(METHOD_EXECUTE_OPERATION, ecuIdAndOpId);
    std::cout << "[GW] ← ExecuteOperation " << r.size() << "B\n";
    return r;
}
