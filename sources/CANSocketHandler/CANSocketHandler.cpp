#include "CANSocketHandler.hpp"
#include "../TimeController/TimeController.hpp"

namespace canftp
{
    void PrintMessage(CanFTP_CanMessage_t* message, uint32_t currentTime, bool rxFlag)
    {
        if (rxFlag)
        {
            printf("Rx :: ");
        }
        else
        {
            printf("Tx :: ");
        }

        printf("<%u> Id= %u, DLC= %u, Data = [%u, %u, %u, %u, %u, %u, %u, %u]\r\n"
            , currentTime
            , message->id
            , message->dataLength
            , message->data[0]
            , message->data[1]
            , message->data[2]
            , message->data[3]
            , message->data[4]
            , message->data[5]
            , message->data[6]
            , message->data[7]);
    }

    bool CANSocketHandler::InitAndRun(std::string interfaceName)
    {
        if (this->IsInited_ != true)
        {
            struct sockaddr_can addr;
            struct ifreq ifr;

            if ((this->Socket_ = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
            {
                return this->IsInited_;
            }

            strcpy(ifr.ifr_name, interfaceName.c_str()); 
            ioctl(this->Socket_, SIOCGIFINDEX, &ifr);

            memset(&addr, 0, sizeof(addr));
            addr.can_family = AF_CAN;
            addr.can_ifindex = ifr.ifr_ifindex;

            if (bind(this->Socket_, (struct sockaddr*)&addr, sizeof(addr)) < 0) 
            {
                return this->IsInited_;
            }

            this->ReceiveMessagesLoopThread_ = std::thread([&]() { this->ReceiveMessagesLoopLogic_(); });

            this->IsInited_ = true;
        }

        return this->IsInited_;
    }

    void CANSocketHandler::SetReceiveMessageHandler(std::function<void (CanFTP_CanMessage_t* message)> eventHandler)
    {
        this->ReceiveMessageEvent_ = eventHandler;
    }

    void CANSocketHandler::SendMessage(CanFTP_CanMessage_t* message)
    {
        if (this->IsInited_)
        {
            can_frame frame;

            frame.can_id = message->id | 0x80000000;
            frame.can_dlc = message->dataLength;

            for (int i = 0; i < message->dataLength; i++)
            {
                frame.data[i] = message->data[i];
            }

            write(this->Socket_, &(frame), sizeof(frame));

            #if (PRINT_TX_MESSAGES)
                PrintMessage(&(message), canftp::time::TimeController::GetCurrentTime(), false);
            #endif
        }
    }

    bool CANSocketHandler::IsActive()
    {
        return this->IsInited_;
    }

    void CANSocketHandler::Dispose()
    {
        if (this->IsInited_)
        {
            close(this->Socket_);
        }
        this->IsInited_ = false;
    }

    void CANSocketHandler::ReceiveMessagesLoopLogic_()
    {
        struct can_frame frame;

        while (this->IsInited_)
        {
            if (read(this->Socket_, &(frame), sizeof(struct can_frame)))
            {
                CanFTP_CanMessage_t message;

                CanFTP_CanMessage_Init(&(message)
                    , (uint32_t)frame.can_id & 0x1FFFFFFF
                    , (uint8_t)frame.can_dlc
                    , (uint8_t*)frame.data);

                #if (PRINT_RX_MESSAGES)
                    PrintMessage(&(message), canftp::time::TimeController::GetCurrentTime(), true);
                #endif

                if (this->ReceiveMessageEvent_ != nullptr)
                {
                    this->ReceiveMessageEvent_(&(message));
                }
            }
        }
    }
}
