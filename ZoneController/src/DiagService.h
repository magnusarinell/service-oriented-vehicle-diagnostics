#pragma once
#include <vsomeip/vsomeip.hpp>

// SOME/IP coordinates — must match HPC/CDA/src/EcuSomeIpClient.h
static constexpr vsomeip::service_t  ECU_SERVICE_ID  = 0x1111;
static constexpr vsomeip::instance_t ECU_INSTANCE_ID = 0x0001;

static constexpr vsomeip::method_t METHOD_GET_CAPABILITIES  = 0x0001;
static constexpr vsomeip::method_t METHOD_READ_DATA         = 0x0002;
static constexpr vsomeip::method_t METHOD_READ_DATA_ITEM    = 0x0003;
static constexpr vsomeip::method_t METHOD_GET_FAULTS        = 0x0004;
static constexpr vsomeip::method_t METHOD_CLEAR_FAULTS      = 0x0005;
static constexpr vsomeip::method_t METHOD_GET_OPERATIONS    = 0x0006;
static constexpr vsomeip::method_t METHOD_EXECUTE_OPERATION = 0x0007;
