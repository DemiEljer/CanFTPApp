#pragma once

#include "../base/BaseEntityHandler.hpp"

extern "C"
{
    #include "CanFTP_Client.h"
};

namespace canftp
{
    class ClientHandler : public BaseEntityHandler<CanFTP_Client_t>
    {
        private: CanFTP_Client_t* Client_()
        {
            return static_cast<CanFTP_Client_t*>(&this->Entity_);
        }

        public: static ClientHandler* GetHandler(CanFTP_Client_t* client)
        {
            return static_cast<ClientHandler*>(GetHandler_(client));
        }

        private: uint8_t* ReceivingFile_ = nullptr;

        private: uint32_t FileLength_ = 0;

        public: void ClientSendMessageCallback(CanFTP_CanMessage_t* message)
        {
            SendMessage_(message);
        }

        public: CanFTP_Logical_t ClientLogicLockRequestCallback();

        public: CanFTP_Logical_t ClientLogicUnlockRequestCallback();

        public: CanFTP_Logical_t ClientSessionConfigurationRequestCallback(CanFTP_Client_Session_Configuration_t* configuration);

        public: void ClientSessionBlockRecievedCallback(CanFTP_FileLength_t firstByteIndex, CanFTP_FileLength_t bytesCount, uint8_t* data);

        public: void ClientSessionHasBeenFinishedCallback(CanFTP_SessionStatus_t status, CanFTP_SoftwareVersion_t* version);

        public: virtual void ReceiveMessage(CanFTP_CanMessage_t* message);

        protected: virtual void InitBase_();

        protected: virtual void InitFromConfigFile_(YAML::Node& config);

        protected: virtual void InitDefault_();

        protected: virtual void LoopLogic_();

        protected: virtual void ConsoleInputLogic_();
    };
}
