#include "ServerHandler.hpp"

namespace canftp
{
    template<>
    std::map<CanFTP_Server_t*, BaseEntityHandler<CanFTP_Server_t>*> BaseEntityHandler<CanFTP_Server_t>::Handlers_ 
        = std::map<CanFTP_Server_t*, BaseEntityHandler<CanFTP_Server_t>*>();

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

    void ServerHandler::ReceiveMessage(CanFTP_CanMessage_t* message)
    {
        CanFTP_Server_RecieveCanMessage(this->Server_(), message);
    }

    void ServerHandler::LoopLogic_()
    {
        CanFTP_Server_Invoke(this->Server_());
    }

    void ServerHandler::ConsoleInputLogic_()
    {
        std::string imputCommand;

        std::cin >> imputCommand;

        if (imputCommand.compare("help") == 0)
        {
            printf("- help      : Print available commands\r\n");
            printf("- info      : Print information about server\r\n");
            printf("- ping      : Star ping process without locking clients logic\r\n");
            printf("- lock      : Star ping process with locking clients logic\r\n");
            printf("- release   : Unlock clients logic\r\n");
            printf("- stop      : Stop ping process (lock,release)\r\n");
            printf("- clients   : Print found clients list\r\n");
            printf("- sessions  : Print created sessions list\r\n");
            printf("- session   : Create a new session\r\n");
            printf("- exit      : Stop server and terminate application\r\n");
        }
        else if (imputCommand.compare("info") == 0)
        {
            printf("= Server information:\r\n");
            printf("RegistrationInterval: %u\r\n", this->Server_()->defaultSessionConfiguration.registrationInterval);
            printf("RegistrationRepeateCount: %u\r\n", this->Server_()->defaultSessionConfiguration.registrationRepeateCount);
            printf("SessionControlInterval: %u\r\n", this->Server_()->defaultSessionConfiguration.sessionControlInterval);
            printf("SessionControlRepeateCount: %u\r\n", this->Server_()->defaultSessionConfiguration.sessionControlRepeateCount);
            printf("RepeateBlockCount: %u\r\n", this->Server_()->defaultSessionConfiguration.repeateBlockCount);
            printf("BlockControlInterval: %u\r\n", this->Server_()->defaultSessionConfiguration.blockControlInterval);
            printf("BlockControlRepeateCount: %u\r\n", this->Server_()->defaultSessionConfiguration.blockControlRepeateCount);
            printf("FrameSendingInterval: %u\r\n", this->Server_()->defaultSessionConfiguration.frameSendingInterval);
            printf("RepeateAckInterval: %u\r\n", this->Server_()->defaultSessionConfiguration.repeateAckInterval );
            printf("RepeateAckCount: %u\r\n", this->Server_()->defaultSessionConfiguration.repeateAckCount);
        }
        else if (imputCommand.compare("ping") == 0)
        {
            printf("= Ping has started\r\n");

            CanFTP_Server_StartPing(this->Server_(), 0);
        }
        else if (imputCommand.compare("lock") == 0)
        {
            printf("= Ping with logic locking has started\r\n");

            CanFTP_Server_StartPing(this->Server_(), 1);
        }
        else if (imputCommand.compare("release") == 0)
        {
            printf("= Release has started\r\n");

            CanFTP_Server_StartRelease(this->Server_());
        }
        else if (imputCommand.compare("stop") == 0)
        {
            printf("= Ping has stoped\r\n");

            CanFTP_Server_StopPing(this->Server_());
        }
        else if (imputCommand.compare("session") == 0)
        {
            bool sessionConfigurationError = false;

            printf("= Session configuration has started\r\n");
            printf("- Input clients count\r\n");

            std::function<bool(int*, std::string)> readNoneZeroPositiveValue = [&](int* value, std::string errorMessage)
            {
                std::cin >> imputCommand;

                try
                {
                    *(value) = std::stoi(imputCommand);
                }
                catch(const std::exception& e)
                {
                    *(value) = -1;
                }

                if (*(value) > 0)
                {
                    return true;
                }
                else
                {
                    printf("! Error: %s\r\n", errorMessage.c_str());

                    sessionConfigurationError = true;

                    return false;
                }
            };

            std::function<bool(int*, std::string)> readNoneNegativeValue = [&](int* value, std::string errorMessage)
            {
                std::cin >> imputCommand;

                try
                {
                    *(value) = std::stoi(imputCommand);
                }
                catch(const std::exception& e)
                {
                    *(value) = -1;
                }

                if (*(value) >= 0)
                {
                    return true;
                }
                else
                {
                    printf("! Error: %s\r\n", errorMessage.c_str());

                    sessionConfigurationError = true;

                    return false;
                }
            };

            int clientsCount = 0;
            int serverClientsCount = CanFTP_Server_GetClientsCount(this->Server_());
            
            if (readNoneZeroPositiveValue(&(clientsCount), "Clients count invalid value"))
            {
                std::unique_ptr<CanFTP_Server_Client_t*[]> sessionClients = std::make_unique<CanFTP_Server_Client_t*[]>(clientsCount);
                // Инициализация клиентов
                for (int i = 0; i < clientsCount; i++)
                {
                    printf("- Input client %u index is server\r\n", i);

                    int clientIndex = -1;

                    if (readNoneNegativeValue(&(clientIndex), "Invalid client index"))
                    {
                        if (clientIndex >= serverClientsCount)
                        {
                            printf("! Error: Client index out of range\r\n");

                            sessionConfigurationError = true;

                            break;
                        }
                        else
                        {
                            CanFTP_Server_Client_t* client = CanFTP_Server_GetClientByIndex(this->Server_(), clientIndex);

                            if (client == CANFTP_NULL)
                            {
                                printf("! Error: No client with such index\r\n");

                                sessionConfigurationError = true;

                                break;
                            }
                            else if (CanFTP_Server_Client_IsInSession(client))
                            {
                                printf("! Error: This client is taken by another session\r\n");

                                sessionConfigurationError = true;

                                break;
                            }
                            else
                            {
                                sessionClients[i] = client;
                            }
                        }
                    }
                }

                CanFTP_SoftwareVersion_t newVersion;
                int versionPartValue = -1;
                int pageIndex = -1;

                if (!sessionConfigurationError)
                {
                    printf("- Input page index\r\n");

                    if (readNoneNegativeValue(&(pageIndex), "Invalid page index"))
                    {
                        newVersion.lowerPart = versionPartValue;
                    }
                }

                if (!sessionConfigurationError)
                {
                    printf("- Input new version: low part\r\n");

                    if (readNoneNegativeValue(&(versionPartValue), "Invalid version value"))
                    {
                        newVersion.lowerPart = versionPartValue;
                    }
                }

                if (!sessionConfigurationError)
                {
                    printf("- Input new version: middle part\r\n");

                    if (readNoneNegativeValue(&(versionPartValue), "Invalid version value"))
                    {
                        newVersion.middlePart = versionPartValue;
                    }
                }

                if (!sessionConfigurationError)
                {
                    printf("- Input new version: high part\r\n");

                    if (readNoneNegativeValue(&(versionPartValue), "Invalid version value"))
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
                    CanFTP_Server_Session_t* session = CanFTP_Server_CreateNewSession(this->Server_());

                    if (session != CANFTP_NULL)
                    {
                        printf("- Session has been created\r\n");

                        this->SessionsFiles_.insert({session, std::make_unique<uint8_t[]>(fileContent.size())});
                        std::copy(fileContent.begin(), fileContent.end(), this->SessionsFiles_[session].get());

                        CanFTP_Server_Session_InitFileConfiguration(session, pageIndex, fileContent.size());
                        printf("- File has been inited\r\n");

                        CanFTP_Server_Session_InitClients(session, clientsCount, sessionClients.get());
                        printf("- Clients have been inited\r\n");

                        CanFTP_Server_Session_InitNewSoftVersion(session, &(newVersion));
                        printf("- New version has been inited\r\n");

                        CanFTP_Server_Session_Start(session);

                        printf("! Session (%u) has started\r\n", session->code);
                    }
                    else
                    {
                        printf("! Session creation error: Maybe ping process is active\r\n");

                        sessionConfigurationError = true;
                    }
                }
            }

            if (sessionConfigurationError)
            {
                printf("! Session error\r\n");
            }
        }
        else if (imputCommand.compare("sessions") == 0)
        {
            printf("= Active sessions:\r\n");

            int sessionsCount = CanFTP_Server_GetActiveSessionsCount(this->Server_());

            for (int i = 0; i < sessionsCount; i++)
            {
                auto session = CanFTP_Server_GetActiveSessionByIndex(this->Server_(), i);

                printf("- Session [%u] : code %u, complete %f %, %u/%u clients, status - %u\r\n"
                    , i
                    , session->code
                    , CanFTP_Server_Session_GetCompletingPercent(session) * 100.0F
                    , CanFTP_Server_Session_GetActiveClientsCount(session)
                    , CanFTP_Server_Session_GetClientsCount(session)
                    , CanFTP_Server_Session_GetStatus(session));

                int clientsCount = CanFTP_Server_Session_GetClientsCount(session);

                for (int j = 0; j < clientsCount; j++)
                {
                    auto client = CanFTP_Server_Session_GetClientByIndex(session, j);

                    printf("    - Client [%u] : serial %u, status %u\r\n"
                        , j
                        , client->serverClient->configuration.serialNumber
                        , client->statuses.sessionStatus);
                }
            }
        }
        else if (imputCommand.compare("clients") == 0)
        {
            printf("= Clients:\r\n");

            int clientsCount = CanFTP_Server_GetClientsCount(this->Server_());

            for (int i = 0; i < clientsCount; i++)
            {
                auto client = CanFTP_Server_GetClientByIndex(this->Server_(), i);

                printf("- Client [%u] : type %u, serial %u, identifier %u, version %u.%u.%u, %s\r\n"
                    , i
                    , client->configuration.type
                    , client->configuration.serialNumber
                    , client->configuration.identifier
                    , client->configuration.softVersion.higherPart, client->configuration.softVersion.middlePart, client->configuration.softVersion.lowerPart
                    , CanFTP_Server_Client_IsInSession(client) ? "buisy" : "free");
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
        CanFTP_Server_Init(this->Server_());

        this->Server_()->controls.doesServerInvokeSessions = CANFTP_TRUE;

        this->Server_()->callbacks.clientFoundCallback = _ServerClientFoundCallback;
        this->Server_()->callbacks.clientReleaseCallback = _ServerClientReleaseCallback;
        this->Server_()->callbacks.getFileBlockCallback = _ServerGetFileBlockCallback;
        this->Server_()->callbacks.sendMessageCallback = _ServerSendMessageCallback;
        this->Server_()->callbacks.sessionFinishedCallback = _ServerSessionFinishedCallback;
    }

    void ServerHandler::InitFromConfigFile_(YAML::Node& config)
    {
        this->Server_()->defaultSessionConfiguration.registrationInterval = std::stoi(config["registrationInterval"].as<std::string>());
        this->Server_()->defaultSessionConfiguration.registrationRepeateCount = std::stoi(config["registrationRepeateCount"].as<std::string>());
        this->Server_()->defaultSessionConfiguration.sessionControlInterval = std::stoi(config["sessionControlInterval"].as<std::string>());
        this->Server_()->defaultSessionConfiguration.sessionControlRepeateCount = std::stoi(config["sessionControlRepeateCount"].as<std::string>());
        this->Server_()->defaultSessionConfiguration.repeateBlockCount = std::stoi(config["repeateBlockCount"].as<std::string>());
        this->Server_()->defaultSessionConfiguration.blockControlInterval = std::stoi(config["blockControlInterval"].as<std::string>());
        this->Server_()->defaultSessionConfiguration.blockControlRepeateCount = std::stoi(config["blockControlRepeateCount"].as<std::string>());
        this->Server_()->defaultSessionConfiguration.frameSendingInterval = std::stoi(config["frameSendingInterval"].as<std::string>());
        this->Server_()->defaultSessionConfiguration.repeateAckInterval = std::stoi(config["repeateAckInterval"].as<std::string>());
        this->Server_()->defaultSessionConfiguration.repeateAckCount = std::stoi(config["repeateAckCount"].as<std::string>());
    }

    void ServerHandler::InitDefault_()
    {

    }
}
