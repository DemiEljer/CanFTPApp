#pragma once

#include <stdio.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include "yaml-cpp/yaml.h"
#include "AbstractEntityHandler.hpp"

namespace canftp
{
    template<class HandlingType>
    class BaseEntityHandler : public AbstractEntityHandler
    {
        protected: static std::map<HandlingType*, BaseEntityHandler<HandlingType>*> Handlers_;

        protected: HandlingType Entity_;

        private: std::function<void (CanFTP_CanMessage_t*)> MessageSendEvent_ = nullptr;

        protected: bool IsActive_ = false;

        private: std::thread LoopThread_;

        private: std::thread ConsoleInputLoopThread_;

        protected: static BaseEntityHandler<HandlingType>* GetHandler_(HandlingType* entity)
        {
            return Handlers_[entity];
        }

        public: ~BaseEntityHandler()
        {
            Stop();
            BaseEntityHandler::Handlers_.erase(&(this->Entity_));
        }

        public: virtual void InitAndRun()
        {
            InitBase_();
            InitDefault_();
            Run_();
        }

        public: virtual void InitAndRun(std::string configurationFile)
        {
            InitBase_();

            try
            {
                YAML::Node config = YAML::LoadFile(configurationFile);

                InitFromConfigFile_(config);

                Run_();
            }
            catch(const std::exception& e)
            {
                printf("!? Configuration file parsing error\r\n");
            }
        }

        public: virtual bool IsActive()
        {
            return IsActive_;
        }

        public: virtual void Stop()
        {
            this->IsActive_ = false;

            this->LoopThread_.~thread();
            this->ConsoleInputLoopThread_.~thread();

            BaseEntityHandler<HandlingType>::Handlers_.erase(&(this->Entity_));
        }

        public: virtual void SetMessageSendEvent(std::function<void (CanFTP_CanMessage_t*)> eventHandler)
        {
            this->MessageSendEvent_ = eventHandler;
        }

        protected: void SendMessage_(CanFTP_CanMessage_t* message)
        {
            if (this->MessageSendEvent_ != nullptr)
            {
                this->MessageSendEvent_(message);
            }
        }

        private: void Run_()
        {
            if (!this->IsActive_)
            {
                this->IsActive_ = true;

                BaseEntityHandler<HandlingType>::Handlers_.insert({&(this->Entity_), this});

                this->LoopThread_ = std::thread([&]() 
                { 
                    while (this->IsActive_)
                    {
                        LoopLogic_(); 

                        std::this_thread::sleep_for(std::chrono::microseconds(500));
                    }
                });
                this->ConsoleInputLoopThread_ = std::thread([&]() 
                { 
                    printf("Input commands to control (help)\r\n");

                    while (this->IsActive_)
                    {
                        ConsoleInputLogic_(); 

                        std::this_thread::sleep_for(std::chrono::microseconds(500));
                    }
                });
            }
        }

        public: virtual void ReceiveMessage(CanFTP_CanMessage_t* message) = 0;

        protected: virtual void InitBase_() = 0;

        protected: virtual void InitFromConfigFile_(YAML::Node &config) = 0;

        protected: virtual void InitDefault_() = 0;

        protected: virtual void LoopLogic_() = 0;

        protected: virtual void ConsoleInputLogic_() = 0;
    };
}
