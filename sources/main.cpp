#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <string.h>

#include <string>
#include "CANSocketHandler/CANSocketHandler.hpp"
#include "TimeController/TimeController.hpp"
#include "ClientHanler/ClientHandler.hpp"
#include "ServerHandler/ServerHandler.hpp"


canftp::can::CANSocketHandler canSocket;
canftp::client::ClientHandler client;
canftp::server::ServerHandler server;

void ErrorHandler(uint32_t errorCode)
{
    printf("The error has occurred : %u\r\n", errorCode);
}


// Флаг, что приложение сейчас активно
std::atomic_bool appliactionIsActive = true;

int main(int argsCount, char** args)
{
    CanFTP_InitErrorHandler(ErrorHandler);
    CanFTP_TimeHandlers_InitCurrentTimeGetter(canftp::time::TimeController::GetCurrentTime);

    if (argsCount < 3)
    {
        printf("Enter the name of interface and application mode (--client or --server)\r\n");

        return 0;
    }

    // Имя интерфейса
    std::string interfaceName = std::string(args[1]);

    std::string configFilePath = "";
    // Определения пути к файлу конфигурации
    if (argsCount >= 4)
    {
        configFilePath = std::string(args[3]);
    }

    bool clientMode = false;

    if (std::strcmp(args[2], "--client") == 0 || std::strcmp(args[2], "-c") == 0)
    {
        // Инициацлизация клиента
        if (configFilePath != "")
        {
            client.InitAndRun(configFilePath);
        }
        else
        {
            client.InitAndRun();
        }

        if (client.IsActive())
        {
            std::cout << "Client has started" << std::endl;
        }

        clientMode = true;
    }
    else if (std::strcmp(args[2], "--server") == 0 || std::strcmp(args[2], "-s") == 0)
    {
        // Инициацлизация клиента
        if (configFilePath != "")
        {
            server.InitAndRun(configFilePath);
        }
        else
        {
            server.InitAndRun();
        }

        if (server.IsActive())
        {
            std::cout << "Server has started" << std::endl;
        }
    }
    else
    {
        printf("Wrong application mode (--client or --server)\r\n");

        return 0;
    }

    client.SetMessageSendEvent([&](CanFTP_CanMessage_t* message) { canSocket.SendMessage(message); });
    server.SetMessageSendEvent([&](CanFTP_CanMessage_t* message) { canSocket.SendMessage(message); });
    // Подключение к сокету CAN
    canSocket.SetReceiveMessageHandler([&](CanFTP_CanMessage_t* message) 
    { 
        if (clientMode)
        {
            client.ReceiveMessage(message); 
        }
        else
        {
            server.ReceiveMessage(message); 
        }
    });
    canSocket.InitAndRun(interfaceName);
    // Не удалось подключиться к сокету
    if (!canSocket.IsActive())
    {
        std::cout << "Can interface connecting error" << std::endl;
    }
    // Запуск контроллера времени
    canftp::time::TimeController::Start();

    // Ожидание окончания потока контроллера клиента
    {
        std::mutex logicLockMutex;
        std::unique_lock<std::mutex> logicLock(logicLockMutex);
        std::condition_variable waiter;

        try
        {
            waiter.wait(logicLock, [&]()
            {
                return (!client.IsActive() && clientMode)
                    || (!server.IsActive() && !clientMode)
                    || !canSocket.IsActive();
            });
        }
        catch(const std::exception& e)
        {

        }
    }

    if (clientMode)
    {
        client.Stop();
    }
    else
    {
        server.Stop();
    }
    canSocket.Dispose();
    canftp::time::TimeController::Stop();

    return 0;
}