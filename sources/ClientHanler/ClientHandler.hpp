#pragma once

#include <stdio.h>
#include <stdio.h>
#include <thread>
#include <chrono>
#include <string>
#include <functional>
#include <fstream>
#include <iostream>
#include <map>
#include "yaml-cpp/yaml.h"

extern "C"
{
    #include "CanFTP_Client.h"
};

namespace canftp
{
namespace client
{
    class ClientHandler
    {
        private: static std::map<CanFTP_Client_t*, ClientHandler*> ClientHandlers_;

        public: static ClientHandler* GetHandler(CanFTP_Client_t* client);

        private: CanFTP_Client_t Client_;

        private: uint8_t* ReceivingFile_ = nullptr;

        private: uint32_t FileLength_ = 0;

        private: std::function<void (CanFTP_CanMessage_t*)> MessageSendEvent_;

        private: bool IsActive_ = false;

        private: std::thread LoopThread_;

        private: std::thread ConsoleInputLoopThread_;

        public: ~ClientHandler();

        public: void InitAndRun();

        public: void InitAndRun(std::string configurationFile);

        public: bool IsActive();

        public: void Stop();

        public: void SetMessageSendEvent(std::function<void (CanFTP_CanMessage_t*)> eventHandler);

        public: void ReceiveMessage(CanFTP_CanMessage_t* message);

        private: void LoopLogic_();

        private: void ConsoleInputLogic_();

        public: void ClientSendMessageCallback(CanFTP_CanMessage_t* message);

        public: CanFTP_Logical_t ClientLogicLockRequestCallback();

        public: CanFTP_Logical_t ClientLogicUnlockRequestCallback();

        public: CanFTP_Logical_t ClientSessionConfigurationRequestCallback(CanFTP_Client_Session_Configuration_t* configuration);

        public: void ClientSessionBlockRecievedCallback(CanFTP_FileLength_t firstByteIndex, CanFTP_FileLength_t bytesCount, uint8_t* data);

        public: void ClientSessionHasBeenFinishedCallback(CanFTP_SessionStatus_t status, CanFTP_SoftwareVersion_t* version);

        private: void InitBase_();

        private: bool InitFromConfigFile_(std::string configurationFile);

        private: void InitDefault_();

        private: void Run_();
    };
}
}
