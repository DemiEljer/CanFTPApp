#include "ServerHandler.hpp"

namespace canftp
{
namespace server
{
    std::map<CanFTP_Server_t*, ServerHandler*> ServerHandler::ServerHandlers_ = std::map<CanFTP_Server_t*, ServerHandler*>();

    void _ServerSendMessageCallback(CanFTP_Server_t* server, CanFTP_CanMessage_t* message)
    {
        ServerHandler::GetHandler(server)->ServerSendMessageCallback(message);
    }

    void _ServerClientFoundCallback(CanFTP_Server_t* server, CanFTP_Server_Client_t* client)
    {
        ServerHandler::GetHandler(server)->ServerClientFoundCallback(client);
    }

    void _ServerSessionFinishedCallback(CanFTP_Server_t* server, CanFTP_Server_Session_t* session, CanFTP_SessionStatus_t status, CanFTP_DeviceCode_t activeClientsCount)
    {
        ServerHandler::GetHandler(server)->ServerSessionFinishedCallback(session, status, activeClientsCount);
    }

    void _ServerGetFileBlockCallback(CanFTP_Server_t* server, CanFTP_Server_Session_t* session, CanFTP_Session_FileBlock_t* fileBlock, CanFTP_FileLength_t startByteIndex, CanFTP_FileLength_t bytesCount)
    {
        ServerHandler::GetHandler(server)->ServerGetFileBlockCallback(session, fileBlock, startByteIndex, bytesCount);
    }

    void _ServerClientReleaseCallback(CanFTP_Server_t* server, CanFTP_Server_Session_t* session, CanFTP_Server_Client_t* client, CanFTP_SessionStatus_t status)
    {
        ServerHandler::GetHandler(server)->ServerClientReleaseCallback(session, client, status);
    }

    ServerHandler* ServerHandler::GetHandler(CanFTP_Server_t* server)
    {
        return ServerHandler::ServerHandlers_[server];
    }

    ServerHandler::~ServerHandler()
    {
        Stop();
        ServerHandler::ServerHandlers_.erase(&(this->Server_));
    }

    void ServerHandler::InitAndRun()
    {
        InitBase_();
        Run_();
    }

    void ServerHandler::InitAndRun(std::string configurationFile)
    {
        InitBase_();
        if (InitFromConfigFile_(configurationFile))
        {
            Run_();
        }
    }

    bool ServerHandler::IsActive()
    {
        return this->IsActive_;
    }

    void ServerHandler::Stop()
    {
        this->IsActive_ = false;
        this->LoopThread_.~thread();
        this->ConsoleInputLoopThread_.~thread();
    }

    void ServerHandler::SetMessageSendEvent(std::function<void (CanFTP_CanMessage_t*)> eventHandler)
    {
        this->MessageSendEvent_ = eventHandler;
    }

    void ServerHandler::ReceiveMessage(CanFTP_CanMessage_t* message)
    {
        CanFTP_Server_RecieveCanMessage(&(this->Server_), message);
    }

    void ServerHandler::LoopLogic_()
    {
        while (this->IsActive_)
        {
            CanFTP_Server_Invoke(&(this->Server_));
            // Вызов логики каждые 0.5 мс
            std::this_thread::sleep_for(std::chrono::microseconds(500));
        }
    }

    void ServerHandler::ConsoleInputLogic_()
    {
        std::string imputCommand;

        printf("Input commands to control server\r\n");

        while (this->IsActive_)
        {
            std::cin >> imputCommand;

            if (imputCommand.compare("ping") == 0)
            {
                printf("= Ping has started\r\n");

                CanFTP_Server_StartPing(&(this->Server_), 1);
            }
            else if (imputCommand.compare("release") == 0)
            {
                printf("= Release has started\r\n");

                CanFTP_Server_StartRelease(&(this->Server_));
            }
            else if (imputCommand.compare("stop") == 0)
            {
                printf("= Ping has stoped\r\n");

                CanFTP_Server_StopPing(&(this->Server_));
            }
            else if (imputCommand.compare("session") == 0)
            {
                bool sessionConfigurationError = false;

                printf("= Session configuration has started\r\n");
                printf("- Input clients count\r\n");

                std::cin >> imputCommand;

                int clientsCount = 0;
                try
                {
                    clientsCount = std::stoi(imputCommand);
                }
                catch(const std::exception& e)
                {
                    clientsCount = 0;
                }

                if (clientsCount <= 0)
                {
                    sessionConfigurationError = true;
                }
                else
                {
                    std::unique_ptr<CanFTP_Server_Client_t*[]> sessionClients = std::make_unique<CanFTP_Server_Client_t*[]>(clientsCount);
                    // Инициализация клиентов
                    for (int i = 0; i < clientsCount; i++)
                    {
                        printf("- Input client %u index is server\r\n", i);

                        std::cin >> imputCommand;

                        int clientIndex = -1;
                        try
                        {
                            clientIndex = std::stoi(imputCommand);
                        }
                        catch(const std::exception& e)
                        {
                            clientIndex = -1;
                        }
                        
                        if (clientIndex < 0)
                        {
                            printf("! Invalid client index\r\n");

                            sessionConfigurationError = true;
                        }
                        else
                        {
                            CanFTP_Server_Client_t* client = CanFTP_Server_GetClientByIndex(&(this->Server_), clientIndex);

                            if (client == CANFTP_NULL)
                            {
                                printf("! No client with such index\r\n");

                                sessionConfigurationError = true;
                            }
                            else
                            {
                                sessionClients[i] = client;
                            }
                        }

                        if (sessionConfigurationError)
                        {
                            break;
                        }
                    }

                    CanFTP_SoftwareVersion_t newVersion;

                    // Инициализация номера версии
                    if (!sessionConfigurationError)
                    {
                        printf("- Input new version: low part\r\n");

                        std::cin >> imputCommand;

                        int versionPartValue = -1;
                        try
                        {
                            versionPartValue = std::stoi(imputCommand);
                        }
                        catch(const std::exception& e)
                        {
                            printf("! Invalid version value\r\n");

                            versionPartValue = -1;
                        }

                        if (versionPartValue < 0)
                        {
                            sessionConfigurationError = true;
                        }
                        else
                        {
                            newVersion.lowerPart = versionPartValue;
                        }
                    }

                    if (!sessionConfigurationError)
                    {
                        printf("- Input new version: middle part\r\n");

                        std::cin >> imputCommand;

                        int versionPartValue = -1;
                        try
                        {
                            versionPartValue = std::stoi(imputCommand);
                        }
                        catch(const std::exception& e)
                        {
                            versionPartValue = -1;
                        }

                        if (versionPartValue < 0)
                        {
                            printf("! Invalid version value\r\n");

                            sessionConfigurationError = true;
                        }
                        else
                        {
                            newVersion.middlePart = versionPartValue;
                        }
                    }

                    if (!sessionConfigurationError)
                    {
                        printf("- Input new version: high part\r\n");

                        std::cin >> imputCommand;

                        int versionPartValue = -1;
                        try
                        {
                            versionPartValue = std::stoi(imputCommand);
                        }
                        catch(const std::exception& e)
                        {
                            versionPartValue = -1;
                        }

                        if (versionPartValue < 0)
                        {
                            printf("! Invalid version value\r\n");

                            sessionConfigurationError = true;
                        }
                        else
                        {
                            newVersion.higherPart = versionPartValue;
                        }
                    }

                    // Инициализация файла
                    std::vector<uint8_t> fileContent;

                    if (!sessionConfigurationError)
                    {
                        printf("- Input path to file\r\n");

                        std::cin >> imputCommand;

                        try
                        {
                            std::ifstream sendingFileStream(imputCommand, std::ios::binary);

                            char byte;
                            while (sendingFileStream.read(&(byte), sizeof(byte)))
                            {
                                fileContent.push_back((uint8_t)byte);
                            }

                            sendingFileStream.close();

                            printf("! File size: %u\r\n", fileContent.size());
                        }
                        catch(const std::exception& e)
                        {
                            printf("! File reading error\r\n");

                            sessionConfigurationError = true;
                        }
                    }
                    // Инициализация и запуск сессии
                    if (!sessionConfigurationError) 
                    {
                        CanFTP_Server_Session_t* session = CanFTP_Server_CreateNewSession(&(this->Server_));
                        printf("- Session created\r\n");

                        this->SessionsFiles_.insert({session, std::make_unique<uint8_t[]>(fileContent.size())});
                        std::copy(fileContent.begin(), fileContent.end(), this->SessionsFiles_[session].get());

                        CanFTP_Server_Session_InitFileConfiguration(session, 0, fileContent.size());
                        printf("- File inited\r\n");

                        CanFTP_Server_Session_InitClients(session, clientsCount, sessionClients.get());
                        printf("- Clients inited\r\n");

                        CanFTP_Server_Session_InitNewSoftVersion(session, &(newVersion));
                        printf("- New version inited\r\n");

                        CanFTP_Server_Session_Start(session);

                        printf("! Session (%u) has started\r\n", session->code);
                    }
                }

                if (sessionConfigurationError)
                {
                    printf("Session error\r\n");
                }
            }
            else if (imputCommand.compare("sessions") == 0)
            {
                printf("= Active sessions:\r\n");

                int sessionsCount = CanFTP_Server_GetActiveSessionsCount(&(this->Server_));

                for (int i = 0; i < sessionsCount; i++)
                {
                    auto session = CanFTP_Server_GetActiveSessionByIndex(&(this->Server_), i);

                    printf("- Session [%u] : code %u, %f %, %u/%u clients, status - %u\r\n"
                        , i
                        , session->code
                        , CanFTP_Server_Session_GetCompletingPercent(session)
                        , CanFTP_Server_Session_GetActiveClientsCount(session)
                        , CanFTP_Server_Session_GetClientsCount(session)
                        , CanFTP_Server_Session_GetStatus(session));
                }
            }
            else if (imputCommand.compare("clients") == 0)
            {
                printf("= Clients:\r\n");

                int clientsCount = CanFTP_Server_GetClientsCount(&(this->Server_));

                for (int i = 0; i < clientsCount; i++)
                {
                    auto client = CanFTP_Server_GetClientByIndex(&(this->Server_), i);

                    printf("- Client [%u] : type %u, serial %u, identifier %u, version %u.%u.%u\r\n"
                        , i
                        , client->configuration.type
                        , client->configuration.serialNumber
                        , client->configuration.identifier
                        , client->configuration.softVersion.higherPart, client->configuration.softVersion.middlePart, client->configuration.softVersion.lowerPart);
                }
            }
            else if (imputCommand.compare("exit") == 0)
            {
                Stop();
            }
            else
            {
                printf("!? Wrong command\r\n");
            }
        }
    }

    void ServerHandler::ServerSendMessageCallback(CanFTP_CanMessage_t* message)
    {
        if (this->MessageSendEvent_ != nullptr)
        {
            this->MessageSendEvent_(message);
        }
    }

    void ServerHandler::ServerClientFoundCallback(CanFTP_Server_Client_t* client)
    {
        printf("! New client has been found\r\n");
        printf(" - Type: %u\r\n", client->configuration.type);
        printf(" - Serial number: %u\r\n", client->configuration.serialNumber);
        printf(" - Identifierr: %u\r\n", client->configuration.identifier);
        printf(" - Version: %u.%u.%u\r\n", client->configuration.softVersion.higherPart, client->configuration.softVersion.middlePart, client->configuration.softVersion.lowerPart);
    }

    void ServerHandler::ServerSessionFinishedCallback(CanFTP_Server_Session_t* session, CanFTP_SessionStatus_t status, CanFTP_DeviceCode_t activeClientsCount)
    {
        printf("! Session (%u) has finished with code: %u\r\n", session->code, status);
        printf("- Clients finished session: %u/%u\r\n"
            , activeClientsCount
            , CanFTP_Server_Session_GetClientsCount(session));

        CanFTP_Server_Session_Delete(session);

        this->SessionsFiles_.erase(session);
    }
        
    void ServerHandler::ServerGetFileBlockCallback(CanFTP_Server_Session_t* session, CanFTP_Session_FileBlock_t* fileBlock, CanFTP_FileLength_t startByteIndex, CanFTP_FileLength_t bytesCount)
    {
        auto file = this->SessionsFiles_[session].get();

        CanFTP_Session_FileBlock_MoveBlockData(fileBlock, (uint8_t*)(file + startByteIndex));
    }
        
    void ServerHandler::ServerClientReleaseCallback(CanFTP_Server_Session_t* session, CanFTP_Server_Client_t* client, CanFTP_SessionStatus_t status)
    {
        printf("! Client has been released from session\r\n");
        printf(" - Serial number: %u\r\n", client->configuration.serialNumber);
        printf(" - Status: %u\r\n", status);
    }

    void ServerHandler::InitBase_()
    {
        CanFTP_Server_Init(&(this->Server_));

        ServerHandler::ServerHandlers_.insert({&(this->Server_), this});

        this->Server_.controls.doesServerInvokeSessions = CANFTP_TRUE;

        this->Server_.callbacks.clientFoundCallback = _ServerClientFoundCallback;
        this->Server_.callbacks.clientReleaseCallback = _ServerClientReleaseCallback;
        this->Server_.callbacks.getFileBlockCallback = _ServerGetFileBlockCallback;
        this->Server_.callbacks.sendMessageCallback = _ServerSendMessageCallback;
        this->Server_.callbacks.sessionFinishedCallback = _ServerSessionFinishedCallback;
    }

    bool ServerHandler::InitFromConfigFile_(std::string configurationFile)
    {
        try
        {
            YAML::Node config = YAML::LoadFile(configurationFile);

            this->Server_.defaultSessionConfiguration.registrationInterval = std::stoi(config["registrationInterval"].as<std::string>());
            this->Server_.defaultSessionConfiguration.registrationRepeateCount = std::stoi(config["registrationRepeateCount"].as<std::string>());
            this->Server_.defaultSessionConfiguration.sessionControlInterval = std::stoi(config["sessionControlInterval"].as<std::string>());
            this->Server_.defaultSessionConfiguration.sessionControlRepeateCount = std::stoi(config["sessionControlRepeateCount"].as<std::string>());
            this->Server_.defaultSessionConfiguration.repeateBlockCount = std::stoi(config["repeateBlockCount"].as<std::string>());
            this->Server_.defaultSessionConfiguration.blockControlInterval = std::stoi(config["blockControlInterval"].as<std::string>());
            this->Server_.defaultSessionConfiguration.blockControlRepeateCount = std::stoi(config["blockControlRepeateCount"].as<std::string>());
            this->Server_.defaultSessionConfiguration.frameSendingInterval = std::stoi(config["frameSendingInterval"].as<std::string>());
            this->Server_.defaultSessionConfiguration.repeateAckInterval = std::stoi(config["repeateAckInterval"].as<std::string>());
            this->Server_.defaultSessionConfiguration.repeateAckCount = std::stoi(config["repeateAckCount"].as<std::string>());

            return true;
        }
        catch(const std::exception& e)
        {
            printf("!? Configuration file parsing error\r\n");

            return false;
        }
    }

    void ServerHandler::Run_()
    {
        if (!this->IsActive_)
        {
            this->IsActive_ = true;

            this->LoopThread_ = std::thread([&]() { LoopLogic_(); });
            this->ConsoleInputLoopThread_ = std::thread([&]() { ConsoleInputLogic_(); });
        }
    }
}
}
