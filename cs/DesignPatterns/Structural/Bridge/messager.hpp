#pragma once

#include <memory>

class MessageSender;

class Message
{
public:
    Message(std::shared_ptr<MessageSender> msg_snder);
    virtual void send() = 0;
protected:
    std::shared_ptr<MessageSender> msg_snder_;
};

class TextMessage : public Message
{
public:
    TextMessage(std::shared_ptr<MessageSender> msg_snder);
    void send() override;
};

class EmailMessage : public Message
{
public:
    EmailMessage(std::shared_ptr<MessageSender> msg_snder);
    void send() override;
};

class MessageSender
{
public:
    virtual void sendMessage() = 0;
};

class TextMessageSender : public MessageSender
{
public:
    void sendMessage() override;
};

class EmailMessageSender : public MessageSender
{
public:
    void sendMessage() override;
};