#include "bedrock/certificates/certificate.h"

#include "endstone/runtime/hook.h"

UnverifiedCertificate UnverifiedCertificate::fromString(const std::string &input)
{
    return ENDSTONE_HOOK_CALL_ORIGINAL(&UnverifiedCertificate::fromString, input);
}

bool Certificate::validate(std::time_t current_time, bool is_self_signed, bool check_expired)
{
    return ENDSTONE_HOOK_CALL_ORIGINAL(&Certificate::validate, this, current_time, is_self_signed, check_expired);
}
