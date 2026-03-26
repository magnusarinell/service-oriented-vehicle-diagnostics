#pragma once
#include <vsomeip/vsomeip.hpp>
#include <string>
#include <functional>
#include <mutex>
#include <condition_variable>

// SOME/IP service coordinates for EcuSimulator
static constexpr vsomeip::service_t  ECU_SERVICE_ID  = 0x1111;
static constexpr vsomeip::instance_t ECU_INSTANCE_ID = 0x0001;

// Method IDs must match EcuSimulator/src/DiagService.h
static constexpr vsomeip::method_t METHOD_GET_CAPABILITIES = 0x0001;
static constexpr vsomeip::method_t METHOD_READ_DATA        = 0x0002;
static constexpr vsomeip::method_t METHOD_READ_DATA_ITEM   = 0x0003;
static constexpr vsomeip::method_t METHOD_GET_FAULTS       = 0x0004;
static constexpr vsomeip::method_t METHOD_CLEAR_FAULTS     = 0x0005;
static constexpr vsomeip::method_t METHOD_GET_OPERATIONS   = 0x0006;
static constexpr vsomeip::method_t METHOD_EXECUTE_OPERATION = 0x0007;

class EcuSomeIpClient {
public:
    explicit EcuSomeIpClient(const std::string& appName = "gateway-ecu");
    ~EcuSomeIpClient();

    // Blocking remote call: serialises request string, returns JSON response string.
    // Thread-safe.
    std::string call(vsomeip::method_t methodId, const std::string& requestJson);

private:
    void onState(vsomeip::state_type_e state);
    void onAvailability(vsomeip::service_t service, vsomeip::instance_t instance, bool available);
    void onResponse(const std::shared_ptr<vsomeip::message>& msg);

    void waitForAvailability();

    std::shared_ptr<vsomeip::application> app_;
    std::thread                           thread_;
    std::mutex                            mtx_;
    std::condition_variable               cv_;
    bool                                  available_{false};
    std::string                           pendingResponse_;
    bool                                  responseReady_{false};
};
