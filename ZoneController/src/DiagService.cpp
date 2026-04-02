#include "DiagService.h"
#include <nlohmann/json.hpp>
#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <functional>

using json = nlohmann::json;

// ── Static diagnostic data ─────────────────────────────────────────────────

static const json CAPABILITIES = json::array({
    {{"name","data"},       {"description","Read live sensor data"},      {"category","read"}},
    {{"name","faults"},     {"description","Read and clear fault codes"}, {"category","read-write"}},
    {{"name","operations"}, {"description","Execute diagnostic routines"},{"category","execute"}},
});

static const json DATA_ITEMS = json::array({
    {{"id","coolant_temp"},    {"name","Coolant Temperature"}, {"value","82"},   {"unit","°C"}},
    {{"id","battery_voltage"}, {"name","Battery Voltage"},     {"value","12.6"}, {"unit","V"}},
    {{"id","engine_rpm"},      {"name","Engine RPM"},          {"value","0"},    {"unit","rpm"}},
    {{"id","vehicle_speed"},   {"name","Vehicle Speed"},       {"value","0"},    {"unit","km/h"}},
});

static const json OPERATIONS = json::array({
    {{"id","reset"},        {"name","ECU Reset"},       {"description","Perform a soft reset of the ECU"}},
    {{"id","self_test"},    {"name","Self-Test"},       {"description","Run internal ECU self-diagnostics"}},
    {{"id","inject_fault"}, {"name","Inject Fault"},    {"description","Inject a fault code (params: code, severity, description)"}},
    {{"id","clear_faults"}, {"name","Clear All Faults"},{"description","Clear all active fault codes"}},
});

// Fault store: starts empty, populated via inject_fault operation
static json activeFaults = json::array();

// ── Helpers ────────────────────────────────────────────────────────────────

static std::string payloadToString(const std::shared_ptr<vsomeip::message>& msg)
{
    auto pl = msg->get_payload();
    return std::string(
        reinterpret_cast<const char*>(pl->get_data()),
        pl->get_length());
}

static void sendResponse(
    std::shared_ptr<vsomeip::application>& app,
    const std::shared_ptr<vsomeip::message>& request,
    const std::string& jsonStr)
{
    auto response = vsomeip::runtime::get()->create_response(request);
    auto payload  = vsomeip::runtime::get()->create_payload();
    std::vector<vsomeip::byte_t> data(jsonStr.begin(), jsonStr.end());
    payload->set_data(data);
    response->set_payload(payload);
    app->send(response);
}

// Split "ecuId:opId:paramsJson" → {ecuId, opId, paramsJson}
static std::tuple<std::string,std::string,std::string> splitArg(const std::string& arg)
{
    auto c1 = arg.find(':');
    if (c1 == std::string::npos) return {arg, "", "{}"};
    auto rest = arg.substr(c1 + 1);
    auto c2 = rest.find(':');
    if (c2 == std::string::npos) return {arg.substr(0, c1), rest, "{}"};
    return {arg.substr(0, c1), rest.substr(0, c2), rest.substr(c2 + 1)};
}

// ── Service class ──────────────────────────────────────────────────────────

class DiagService {
public:
    DiagService()
        : app_(vsomeip::runtime::get()->create_application("ecm"))
    {}

    bool init()
    {
        if (!app_->init()) {
            std::cerr << "[ECM] Failed to init vsomeip\n";
            return false;
        }

        app_->register_state_handler(
            [this](vsomeip::state_type_e s){ onState(s); });

        // Register one handler per method
        auto reg = [&](vsomeip::method_t m, auto fn){
            app_->register_message_handler(
                ECU_SERVICE_ID, ECU_INSTANCE_ID, m, fn);
        };

        reg(METHOD_GET_CAPABILITIES,  [this](auto& r){ onGetCapabilities(r); });
        reg(METHOD_READ_DATA,         [this](auto& r){ onReadData(r); });
        reg(METHOD_READ_DATA_ITEM,    [this](auto& r){ onReadDataItem(r); });
        reg(METHOD_GET_FAULTS,        [this](auto& r){ onGetFaults(r); });
        reg(METHOD_CLEAR_FAULTS,      [this](auto& r){ onClearFaults(r); });
        reg(METHOD_GET_OPERATIONS,    [this](auto& r){ onGetOperations(r); });
        reg(METHOD_EXECUTE_OPERATION, [this](auto& r){ onExecuteOperation(r); });

        return true;
    }

    void start() { app_->start(); }

private:
    void onState(vsomeip::state_type_e state)
    {
        if (state == vsomeip::state_type_e::ST_REGISTERED) {
            app_->offer_service(ECU_SERVICE_ID, ECU_INSTANCE_ID);
            std::cout << "[ECM] Service offered (0x"
                      << std::hex << ECU_SERVICE_ID << "/0x"
                      << ECU_INSTANCE_ID << std::dec << ")\n";
        }
    }

    void onGetCapabilities(const std::shared_ptr<vsomeip::message>& req)
    {
        std::cout << "[ECM] ← GetCapabilities\n";
        sendResponse(app_, req, CAPABILITIES.dump());
    }

    void onReadData(const std::shared_ptr<vsomeip::message>& req)
    {
        std::cout << "[ECM] ← ReadData\n";
        sendResponse(app_, req, DATA_ITEMS.dump());
    }

    void onReadDataItem(const std::shared_ptr<vsomeip::message>& req)
    {
        auto arg    = payloadToString(req);
        auto colon  = arg.find(':');
        auto dataId = colon != std::string::npos ? arg.substr(colon + 1) : arg;
        std::cout << "[ECM] ← ReadDataItem(" << dataId << ")\n";

        for (auto& item : DATA_ITEMS) {
            if (item["id"].get<std::string>() == dataId) {
                sendResponse(app_, req, item.dump());
                return;
            }
        }
        sendResponse(app_, req, "null");
    }

    void onGetFaults(const std::shared_ptr<vsomeip::message>& req)
    {
        std::cout << "[ECM] ← GetFaults (" << activeFaults.size() << " active)\n";
        sendResponse(app_, req, activeFaults.dump());
    }

    void onClearFaults(const std::shared_ptr<vsomeip::message>& req)
    {
        std::cout << "[ECM] ← ClearFaults\n";
        activeFaults = json::array();
        sendResponse(app_, req, R"({"status":"ok"})");
    }

    void onGetOperations(const std::shared_ptr<vsomeip::message>& req)
    {
        std::cout << "[ECM] ← GetOperations\n";
        sendResponse(app_, req, OPERATIONS.dump());
    }

    void onExecuteOperation(const std::shared_ptr<vsomeip::message>& req)
    {
        auto arg = payloadToString(req);
        auto [ecuId, opId, paramsStr] = splitArg(arg);
        std::cout << "[ECM] ← ExecuteOperation(" << opId << ") params=" << paramsStr << "\n";

        json result;
        if (opId == "reset") {
            result = {{"status","ok"},{"message","ECU reset acknowledged"}};
        } else if (opId == "self_test") {
            result = {{"status","ok"},{"message","Self-test passed"},
                      {"outputParameters",{{"result","pass"}}}};
        } else if (opId == "inject_fault") {
            try {
                auto params = json::parse(paramsStr);
                std::string code  = params.value("code", "PXXXX");
                std::string sev   = params.value("severity", "medium");
                std::string desc  = params.value("description", "Injected fault");
                // Remove existing entry with same code, then add
                activeFaults.erase(std::remove_if(activeFaults.begin(), activeFaults.end(),
                    [&](const json& f){ return f.value("code","") == code; }), activeFaults.end());
                activeFaults.push_back({{"code",code},{"description",desc},{"severity",sev},{"status","active"}});
                std::cout << "[ECM] Injected fault " << code << "\n";
                result = {{"status","ok"},{"message","Fault " + code + " injected"}};
            } catch (const std::exception& e) {
                result = {{"status","error"},{"message",std::string("Bad params: ") + e.what()}};
            }
        } else if (opId == "clear_faults") {
            activeFaults = json::array();
            std::cout << "[ECM] Cleared all faults\n";
            result = {{"status","ok"},{"message","All faults cleared"}};
        } else {
            result = {{"status","error"},{"message","Unknown operation: " + opId}};
        }
        sendResponse(app_, req, result.dump());
    }

    std::shared_ptr<vsomeip::application> app_;
};
