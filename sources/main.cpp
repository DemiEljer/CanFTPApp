#include <mutex>
#include <condition_variable>

#include "CANSocketHandler/CANSocketHandler.hpp"
#include "TimeController/TimeController.hpp"
#include "ClientHandler/ClientHandler.hpp"
#include "ServerHandler/ServerHandler.hpp"

// Режим работы в качестве клиента
#define CLIENT_MODE 1
// Режим работы в качестве сервера
#define SERVER_MODE 2

canftp::CANSocketHandler canSocket;
canftp::AbstractEntityHandler* canFTPHandler = nullptr;

// Флаг вывода help
bool helpShow = false;
// Имя интерфейса
std::string interfaceName = "";
// Режим работы приложения
int applicationMode = 0;
// Путь к конфигурационному файлу
std::string configFilePath = "";

void ErrorHandler(uint32_t errorCode)
{
    printf("The error has occurred : %u\r\n", errorCode);

    if (canFTPHandler != nullptr)
    {
        canFTPHandler->Stop();
    }
}

bool argsReading(int argsCount, char** args)
{
    if (argsCount == 2)
    {
        if (std::strcmp(args[1], "--help") == 0 || std::strcmp(args[1], "-h") == 0)
        {
            helpShow = true;

            return true;
        }
    }
    else if (argsCount == 3 || argsCount == 4)
    {
        interfaceName = args[1];

        if (std::strcmp(args[2], "--client") == 0 || std::strcmp(args[2], "-c") == 0)
        {
            applicationMode = CLIENT_MODE;
        }
        else if (std::strcmp(args[2], "--server") == 0 || std::strcmp(args[2], "-s") == 0)
        {
            applicationMode = SERVER_MODE;
        }

        if (argsCount == 4)
        {
            configFilePath = args[3];
        }

        return true;
    }

    return false;
}

void printHelp()
{
    printf("Application invoke arguments combinations:\r\n");
    printf("... <-h|--help>                         : print application help\r\n");
    printf("... <interface> <mode> [configFile]     : application start up\r\n");
    printf("    - <interface>                       : the name of CAN interface\r\n");
    printf("    - <mode>                            : -c|--client (client mode) or -s|--server (server mode)\r\n");
    printf("    - [configFile]                      : path to config file (.yaml)\r\n");
    printf("Examples:\r\n");
    printf("./CanFTPApp -h\r\n");
    printf("sudo ./CanFTPApp vcan0 -s\r\n");
    printf("sudo ./CanFTPApp vcan0 --client ./clientConfiguration.yaml\r\n");
}

int main(int argsCount, char** args)
{
    printf("CanFTPApp (v1.0.1)\r\n");

    // Чтение аргументов вызова
    {
        if (!argsReading(argsCount, args)
            || helpShow)
        {
            printHelp();

            return 0;
        }
    }
    // Базовая инициализация
    {
        CanFTP_InitErrorHandler(ErrorHandler);
        CanFTP_TimeHandlers_InitCurrentTimeGetter(canftp::TimeController::GetCurrentTime);
    }
    // Создание обработчика
    {
        if (applicationMode == CLIENT_MODE)
        {
            canFTPHandler = (canftp::AbstractEntityHandler*)(new canftp::ClientHandler());

            printf("Application started in client mode\r\n");
        }
        else if (applicationMode == SERVER_MODE)
        {
            canFTPHandler = (canftp::AbstractEntityHandler*)(new canftp::ServerHandler());

            printf("Application started in server mode\r\n");
        }
    }
    // Инициализация обработчика
    {
        if (configFilePath != "")
        {
            canFTPHandler->InitAndRun(configFilePath);
        }
        else
        {
            canFTPHandler->InitAndRun();
        }
    }
    // Инциализация и запуск контроллеров CAN и времени
    {
        canFTPHandler->SetMessageSendEvent([&](CanFTP_CanMessage_t* message) { canSocket.SendMessage(message); });
        // Подключение к сокету CAN
        canSocket.SetReceiveMessageHandler([&](CanFTP_CanMessage_t* message) 
        { 
            canFTPHandler->ReceiveMessage(message);
        });

        canSocket.InitAndRun(interfaceName);
        // Не удалось подключиться к сокету
        if (!canSocket.IsActive())
        {
            std::cout << "Can interface connecting error" << std::endl;
        }

        // Запуск контроллера времени
        canftp::TimeController::Start();
    }
    // Ожидание окончания обработчика или контроллера CAN
    {
        std::mutex logicLockMutex;
        std::unique_lock<std::mutex> logicLock(logicLockMutex);
        std::condition_variable waiter;

        waiter.wait(logicLock, [&]()
        {
            return     !canFTPHandler->IsActive()
                    || !canSocket.IsActive();
        });
    }
    // Остановка приложения
    {
        canFTPHandler->Stop();
        delete canFTPHandler;
        canSocket.Dispose();
        canftp::TimeController::Stop();
    }

    return 0;
}