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
#include <vector>
#include <iterator>

extern "C"
{
    #include "CanFTP_Server.h"
};

namespace canftp
{
namespace server
{
    class ServerHandler
    {
        private: static std::map<CanFTP_Server_t*, ServerHandler*> ServerHandlers_;

        public: static ServerHandler* GetHandler(CanFTP_Server_t* server);

        private: CanFTP_Server_t Server_;

        private: std::map<CanFTP_Server_Session_t*, std::unique_ptr<uint8_t[]>> SessionsFiles_;

        private: std::function<void (CanFTP_CanMessage_t*)> MessageSendEvent_;

        private: bool IsActive_ = false;

        private: std::thread LoopThread_;

        private: std::thread ConsoleInputLoopThread_;

        public: ~ServerHandler();

        public: void InitAndRun();

        public: void InitAndRun(std::string configurationFile);

        public: bool IsActive();

        public: void Stop();

        public: void SetMessageSendEvent(std::function<void (CanFTP_CanMessage_t*)> eventHandler);

        public: void ReceiveMessage(CanFTP_CanMessage_t* message);

        private: void LoopLogic_();

        private: void ConsoleInputLogic_();

        public: void ServerSendMessageCallback(CanFTP_CanMessage_t* message);

        public: void ServerClientFoundCallback(CanFTP_Server_Client_t* client);

        public: void ServerSessionFinishedCallback(CanFTP_Server_Session_t* session, CanFTP_SessionStatus_t status, CanFTP_DeviceCode_t activeClientsCount);
        
        public: void ServerGetFileBlockCallback(CanFTP_Server_Session_t* session, CanFTP_Session_FileBlock_t* fileBlock, CanFTP_FileLength_t startByteIndex, CanFTP_FileLength_t bytesCount);
        
        public: void ServerClientReleaseCallback(CanFTP_Server_Session_t* session, CanFTP_Server_Client_t* client, CanFTP_SessionStatus_t status);

        private: void InitBase_();

        private: bool InitFromConfigFile_(std::string configurationFile);

        private: void InitDefault_();

        private: void Run_();
    };
}
}
