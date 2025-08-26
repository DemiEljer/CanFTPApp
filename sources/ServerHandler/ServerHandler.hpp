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
#include "../base/BaseEntityHandler.hpp"

extern "C"
{
    #include "CanFTP_Server.h"
};

namespace canftp
{
class ServerHandler : public BaseEntityHandler<CanFTP_Server_t>
{
    private: CanFTP_Server_t* Server_()
    {
        return static_cast<CanFTP_Server_t*>(&this->Entity_);
    }

    public: static ServerHandler* GetHandler(CanFTP_Server_t* server)
    {
        return static_cast<ServerHandler*>(GetHandler_(server));
    }

    private: std::map<CanFTP_Server_Session_t*, std::unique_ptr<uint8_t[]>> SessionsFiles_;

    public: void ServerSendMessageCallback(CanFTP_CanMessage_t* message)
    {
        SendMessage_(message);
    }

    public: void ServerClientFoundCallback(CanFTP_Server_Client_t* client);

    public: void ServerSessionFinishedCallback(CanFTP_Server_Session_t* session, CanFTP_SessionStatus_t status, CanFTP_DeviceCode_t activeClientsCount);
    
    public: void ServerGetFileBlockCallback(CanFTP_Server_Session_t* session, CanFTP_Session_FileBlock_t* fileBlock, CanFTP_FileLength_t startByteIndex, CanFTP_FileLength_t bytesCount);
    
    public: void ServerClientReleaseCallback(CanFTP_Server_Session_t* session, CanFTP_Server_Client_t* client, CanFTP_SessionStatus_t status);

    public: virtual void ReceiveMessage(CanFTP_CanMessage_t* message);

    protected: virtual void InitBase_();

    protected: virtual void InitFromConfigFile_(YAML::Node& config);

    protected: virtual void InitDefault_();

    protected: virtual void LoopLogic_();

    protected: virtual void ConsoleInputLogic_();
};
}
