#include "GatewayDbusService.h"

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
    return someip_->call(METHOD_GET_CAPABILITIES, ecuId);
}

std::string GatewayDbusService::ReadData(const std::string& ecuId)
{
    return someip_->call(METHOD_READ_DATA, ecuId);
}

std::string GatewayDbusService::ReadDataItem(const std::string& ecuIdAndDataId)
{
    return someip_->call(METHOD_READ_DATA_ITEM, ecuIdAndDataId);
}

std::string GatewayDbusService::GetFaults(const std::string& ecuId)
{
    return someip_->call(METHOD_GET_FAULTS, ecuId);
}

std::string GatewayDbusService::ClearFaults(const std::string& ecuId)
{
    someip_->call(METHOD_CLEAR_FAULTS, ecuId);
    return R"({"status":"ok"})";
}

std::string GatewayDbusService::GetOperations(const std::string& ecuId)
{
    return someip_->call(METHOD_GET_OPERATIONS, ecuId);
}

std::string GatewayDbusService::ExecuteOperation(const std::string& ecuIdAndOpId)
{
    return someip_->call(METHOD_EXECUTE_OPERATION, ecuIdAndOpId);
}
