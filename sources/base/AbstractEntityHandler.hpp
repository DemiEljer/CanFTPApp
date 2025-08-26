#pragma once

#include <string>
#include <functional>

extern "C" 
{
    #include "CanFTP_Messages_Hub.h"
};

namespace canftp
{
    class AbstractEntityHandler
    {
        public: virtual void InitAndRun() = 0;

        public: virtual void InitAndRun(std::string configurationFile) = 0;

        public: virtual bool IsActive() = 0;

        public: virtual void Stop() = 0;

        public: virtual void SetMessageSendEvent(std::function<void (CanFTP_CanMessage_t*)> eventHandler) = 0;

        public: virtual void ReceiveMessage(CanFTP_CanMessage_t* message) = 0;
    };
}
