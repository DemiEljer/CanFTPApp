#pragma once

#include <stdlib.h>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <functional>
#include <atomic>

extern "C"
{
    #include "CanFTP_CanMessage.h"
}

#include <string>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#define PRINT_RX_MESSAGES 0x00
#define PRINT_TX_MESSAGES 0x00

namespace canftp
{
    class CANSocketHandler
    {
        private: int Socket_ = 0;

        private: std::atomic_bool IsInited_ = false;

        private: std::function<void (CanFTP_CanMessage_t* message)> ReceiveMessageEvent_ = nullptr;

        private: std::thread ReceiveMessagesLoopThread_;

        public: bool InitAndRun(std::string interfaceName);

        public: void SetReceiveMessageHandler(std::function<void (CanFTP_CanMessage_t* message)> eventHandler);

        public: void SendMessage(CanFTP_CanMessage_t* message);

        public: bool IsActive();

        public: void Dispose();

        private: void ReceiveMessagesLoopLogic_();
    };
}
