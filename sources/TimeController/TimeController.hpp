#pragma once

#include <thread>
#include <chrono>
extern "C"
{
    #include "CanFTP_TimeHandlers.h"
}

namespace canftp
{
namespace time
{
    class TimeController
    {
        private: static CanFTP_TimeMark_t CurrentTime_;

        private: static bool IsActive_;

        private: static std::thread* TimeThread_;

        public: static void Start();

        public: static void Stop();

        public: static CanFTP_TimeMark_t GetCurrentTime();
        
        private: static void TimeThreadLogic_();
    };
}
}
