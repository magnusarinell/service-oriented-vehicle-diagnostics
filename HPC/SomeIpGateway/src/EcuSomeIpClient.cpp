#include "EcuSomeIpClient.h"
#include <iostream>
#include <thread>
#include <chrono>

EcuSomeIpClient::EcuSomeIpClient(const std::string& appName)
    : app_(vsomeip::runtime::get()->create_application(appName))
{
    if (!app_->init()) {
        throw std::runtime_error("vsomeip: failed to initialise application");
    }

    app_->register_state_handler(
        [this](vsomeip::state_type_e s){ onState(s); });

    app_->register_availability_handler(
        ECU_SERVICE_ID, ECU_INSTANCE_ID,
        [this](vsomeip::service_t svc, vsomeip::instance_t inst, bool avail){
            onAvailability(svc, inst, avail);
        });

    // Single response handler for all methods
    app_->register_message_handler(
        vsomeip::ANY_SERVICE, vsomeip::ANY_INSTANCE, vsomeip::ANY_METHOD,
        [this](const std::shared_ptr<vsomeip::message>& msg){ onResponse(msg); });

    thread_ = std::thread([this]{ app_->start(); });
}

EcuSomeIpClient::~EcuSomeIpClient()
{
    app_->release_service(ECU_SERVICE_ID, ECU_INSTANCE_ID);
    app_->clear_all_handler();
    app_->stop();
    if (thread_.joinable()) thread_.join();
}

std::string EcuSomeIpClient::call(vsomeip::method_t methodId, const std::string& requestJson)
{
    waitForAvailability();

    auto request = vsomeip::runtime::get()->create_request();
    request->set_service(ECU_SERVICE_ID);
    request->set_instance(ECU_INSTANCE_ID);
    request->set_method(methodId);
    request->set_reliable(true);

    auto payload = vsomeip::runtime::get()->create_payload();
    std::vector<vsomeip::byte_t> data(requestJson.begin(), requestJson.end());
    payload->set_data(data);
    request->set_payload(payload);

    {
        std::unique_lock<std::mutex> lk(mtx_);
        responseReady_ = false;
        pendingResponse_.clear();
    }

    app_->send(request);

    // Wait for response (5 s timeout)
    std::unique_lock<std::mutex> lk(mtx_);
    if (!cv_.wait_for(lk, std::chrono::seconds(5), [this]{ return responseReady_; }))
        return R"({"error":"timeout"})";

    return pendingResponse_;
}

void EcuSomeIpClient::onState(vsomeip::state_type_e state)
{
    if (state == vsomeip::state_type_e::ST_REGISTERED)
        app_->request_service(ECU_SERVICE_ID, ECU_INSTANCE_ID);
}

void EcuSomeIpClient::onAvailability(
    vsomeip::service_t, vsomeip::instance_t, bool available)
{
    std::unique_lock<std::mutex> lk(mtx_);
    available_ = available;
    if (available_) cv_.notify_all();
}

void EcuSomeIpClient::onResponse(const std::shared_ptr<vsomeip::message>& msg)
{
    auto pl = msg->get_payload();
    std::string json(
        reinterpret_cast<const char*>(pl->get_data()),
        pl->get_length());
    {
        std::unique_lock<std::mutex> lk(mtx_);
        pendingResponse_ = std::move(json);
        responseReady_   = true;
    }
    cv_.notify_one();
}

void EcuSomeIpClient::waitForAvailability()
{
    std::unique_lock<std::mutex> lk(mtx_);
    cv_.wait_for(lk, std::chrono::seconds(10), [this]{ return available_; });
}
