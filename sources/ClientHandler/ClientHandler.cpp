#include "ClientHandler.hpp"

namespace canftp
{
    template<>
    std::map<CanFTP_Client_t*, BaseEntityHandler<CanFTP_Client_t>*> BaseEntityHandler<CanFTP_Client_t>::Handlers_ 
        = std::map<CanFTP_Client_t*, BaseEntityHandler<CanFTP_Client_t>*>();

    static void _ClientSendMessageCallback(CanFTP_Client_t* client, CanFTP_CanMessage_t* message)
    {
        ClientHandler::GetHandler(client)->ClientSendMessageCallback(message);
    }

    static CanFTP_Logical_t _ClientLogicLockRequestCallback(CanFTP_Client_t* client)
    {
        return ClientHandler::GetHandler(client)->ClientLogicLockRequestCallback();
    }

    static CanFTP_Logical_t _ClientLogicUnlockRequestCallback(CanFTP_Client_t* client)
    {
        return ClientHandler::GetHandler(client)->ClientLogicUnlockRequestCallback();
    }

    static CanFTP_Logical_t _ClientSessionConfigurationRequestCallback(CanFTP_Client_t* client, CanFTP_Client_Session_Configuration_t* configuration)
    {
        return ClientHandler::GetHandler(client)->ClientSessionConfigurationRequestCallback(configuration);
    }

    static void _ClientSessionBlockRecievedCallback(CanFTP_Client_t* client, CanFTP_FileLength_t firstByteIndex, CanFTP_FileLength_t bytesCount, uint8_t* data)
    {
        ClientHandler::GetHandler(client)->ClientSessionBlockRecievedCallback(firstByteIndex, bytesCount, data);
    }

    static void _ClientSessionHasBeenFinishedCallback(CanFTP_Client_t* client, CanFTP_SessionStatus_t status, CanFTP_SoftwareVersion_t* version)
    {
        ClientHandler::GetHandler(client)->ClientSessionHasBeenFinishedCallback(status, version);
    }

    void ClientHandler::ReceiveMessage(CanFTP_CanMessage_t* message)
    {
        CanFTP_Client_RecieveCanMessage(this->Client_(), message);
    }

    void ClientHandler::LoopLogic_()
    {
        CanFTP_Client_Invoke(this->Client_());
    }

    void ClientHandler::ConsoleInputLogic_()
    {
        std::string imputCommand;

        std::cin >> imputCommand;

        if (imputCommand.compare("help") == 0)
        {
            printf("- help : Print available commands\r\n");
            printf("- info : Print information about client\r\n");
            printf("- exit : Stop client and terminate application\r\n");
        }
        else if (imputCommand.compare("info") == 0)
        {
            printf("= Client information:\r\n");
            printf(" - Type: %u\r\n", this->Client_()->deviceConfig.type);
            printf(" - Serial number: %u\r\n", this->Client_()->deviceConfig.serialNumber);
            printf(" - Identifierr: %u\r\n", this->Client_()->deviceConfig.identifier);
            printf(" - Version: %u.%u.%u\r\n"
                , this->Client_()->deviceConfig.softVersion.higherPart
                , this->Client_()->deviceConfig.softVersion.middlePart
                , this->Client_()->deviceConfig.softVersion.lowerPart);
        }
        else if (imputCommand.compare("exit") == 0)
        {
            Stop();
        }
        else
        {
            printf("!? Wrong command (help)\r\n");
        }
    }

    CanFTP_Logical_t ClientHandler::ClientLogicLockRequestCallback()
    {
        printf("! Client (%u) has been locked\r\n", this->Client_()->deviceConfig.serialNumber);

        return CANFTP_TRUE;
    }

    CanFTP_Logical_t ClientHandler::ClientLogicUnlockRequestCallback()
    {
        printf("! Client (%u) has been unlocked\r\n", this->Client_()->deviceConfig.serialNumber);

        return CANFTP_TRUE;
    }

    CanFTP_Logical_t ClientHandler::ClientSessionConfigurationRequestCallback(CanFTP_Client_Session_Configuration_t* configuration)
    {
        printf("! Client (%u) has been configured for session\r\n", this->Client_()->deviceConfig.serialNumber);
        printf("- PageIndex = %u\r\n", configuration->pageIndex);
        printf("- FileLength = %u\r\n", configuration->fileLength);
        printf("- RepeateAckCount = %u\r\n", configuration->repeateAckCount);
        printf("- RepeateInterval = %u\r\n", configuration->repeateInterval);

        if (this->ReceivingFile_ != nullptr)
        {
            delete [] this->ReceivingFile_;
        }

        this->FileLength_ = configuration->fileLength;
        this->ReceivingFile_ = new uint8_t[configuration->fileLength];

        return CANFTP_TRUE;
    }

    void ClientHandler::ClientSessionBlockRecievedCallback(CanFTP_FileLength_t firstByteIndex, CanFTP_FileLength_t bytesCount, uint8_t* data)
    {
        if (this->ReceivingFile_ != nullptr)
        {
            for (int i = 0; i < bytesCount; i++)
            {
                this->ReceivingFile_[firstByteIndex + i] = data[i];
            }
        }
    }

    void ClientHandler::ClientSessionHasBeenFinishedCallback(CanFTP_SessionStatus_t status, CanFTP_SoftwareVersion_t* version)
    {
        if (status == CANFTP_SESSIONSTATUS_OK)
        {
            printf("! Client (%u) has successfuly finished session\r\n", this->Client_()->deviceConfig.serialNumber);
            printf("- New version : %u.%u.%u\r\n", version->higherPart, version->middlePart, version->lowerPart);
            // Сохранение полученного файла в файл
            {
                std::ofstream recievedFileStream(std::to_string(this->Client_()->deviceConfig.serialNumber) + "_receivedFile.txt", std::ios::out | std::ios::binary);

                for (int i = 0; i < this->FileLength_; i++)
                {
                    recievedFileStream << this->ReceivingFile_[i];
                }
                recievedFileStream.close();
            }
        }
        else
        {
            printf("! Client (%u) has failed session with status %u\r\n", this->Client_()->deviceConfig.serialNumber, status);
        }
        // Удаление файла
        if (this->ReceivingFile_ != nullptr)
        {
            delete [] this->ReceivingFile_;
            this->ReceivingFile_ = nullptr;
            
            this->FileLength_ = 0;
        }
    }

    void ClientHandler::InitBase_()
    {
        CanFTP_Client_Init(this->Client_());

        this->Client_()->callbacks.blockRecieceCallback = _ClientSessionBlockRecievedCallback;
        this->Client_()->callbacks.lockLogicRequestCallback = _ClientLogicLockRequestCallback;
        this->Client_()->callbacks.sendMessageCallback = _ClientSendMessageCallback;
        this->Client_()->callbacks.sessionConfigureationCallback = _ClientSessionConfigurationRequestCallback;
        this->Client_()->callbacks.sessionFinishedCallback = _ClientSessionHasBeenFinishedCallback;
        this->Client_()->callbacks.unlockLogicRequestCallback = _ClientLogicUnlockRequestCallback;
    }

    void ClientHandler::InitFromConfigFile_(YAML::Node& config)
    {
        this->Client_()->control.pingPermition = std::stoi(config["pingPermition"].as<std::string>());
        this->Client_()->control.sessionStartPermition = std::stoi(config["sessionStartPermition"].as<std::string>());
        this->Client_()->control.autpUpdateSoftVersion = std::stoi(config["autpUpdateSoftVersion"].as<std::string>());
        this->Client_()->deviceConfig.serialNumber = std::stoi(config["serialNumber"].as<std::string>());
        this->Client_()->deviceConfig.identifier = std::stoi(config["identifier"].as<std::string>());
        this->Client_()->deviceConfig.type = std::stoi(config["type"].as<std::string>());
        this->Client_()->deviceConfig.softVersion.lowerPart = std::stoi(config["softLowerPart"].as<std::string>());
        this->Client_()->deviceConfig.softVersion.middlePart = std::stoi(config["softMiddlePart"].as<std::string>());
        this->Client_()->deviceConfig.softVersion.higherPart = std::stoi(config["softHigherPart"].as<std::string>());
    }

    void ClientHandler::InitDefault_()
    {
        this->Client_()->control.pingPermition = CANFTP_TRUE;
        this->Client_()->control.sessionStartPermition = CANFTP_TRUE;
        this->Client_()->control.autpUpdateSoftVersion = CANFTP_TRUE;
        this->Client_()->deviceConfig.serialNumber = rand();
        this->Client_()->deviceConfig.identifier = rand() & 0xFF;
        this->Client_()->deviceConfig.type = rand() & 0xFFFFF;
        this->Client_()->deviceConfig.softVersion.lowerPart = rand() & 0xFF;
        this->Client_()->deviceConfig.softVersion.middlePart = rand() & 0xFF;
        this->Client_()->deviceConfig.softVersion.higherPart = rand() & 0xFF;
    }
}