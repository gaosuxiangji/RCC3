#include "agerrorcode.h"

#include "Private/agerrorcodeprivate.h"

AgErrorCode::AgErrorCode() : d_ptr_(new AgErrorCodePrivate)
{

}

AgErrorCode &AgErrorCode::instance()
{
    static AgErrorCode inst;
    return inst;
}

AgErrorCode::~AgErrorCode()
{
    if (d_ptr_)
    {
        delete d_ptr_;
        d_ptr_ = nullptr;
    }
}

bool AgErrorCode::reset(int lang)
{
    return d_ptr_->reset(lang);
}

bool AgErrorCode::get(uint32_t value, AgErrorCodeInfo &info)
{
    return d_ptr_->get(value, info);
}
