#include "TimeController.hpp"

namespace canftp
{
    CanFTP_TimeMark_t TimeController::CurrentTime_ = 0;
    bool TimeController::IsActive_ = false;
    std::thread* TimeController::TimeThread_ = nullptr;

    void TimeController::Start()
    {
        if (!TimeController::IsActive_
            && TimeController::TimeThread_ == nullptr)
        {
            TimeController::TimeThread_ = new std::thread([]() { TimeController::TimeThreadLogic_(); });

            TimeController::IsActive_ = true;
        }
    }

    void TimeController::Stop()
    {
        TimeController::IsActive_ = false;
    }

    CanFTP_TimeMark_t TimeController::GetCurrentTime()
    {
        return TimeController::CurrentTime_;
    }
    
    void TimeController::TimeThreadLogic_()
    {
        while (TimeController::IsActive_)
        {
            // Вызов логики каждые 1 мс
            std::this_thread::sleep_for(std::chrono::microseconds(1000));

            TimeController::CurrentTime_++;
        }

        delete TimeController::TimeThread_;
        TimeController::TimeThread_ = nullptr;
    }
}
