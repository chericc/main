#include "messager.hpp"

#include <iostream>

Message::Message(std::shared_ptr<MessageSender> msg_snder) {
    msg_snder_ = msg_snder;
}

TextMessage::TextMessage(std::shared_ptr<MessageSender> msg_snder)
    : Message(msg_snder) {}

void TextMessage::send() { msg_snder_->sendMessage(); }

EmailMessage::EmailMessage(std::shared_ptr<MessageSender> msg_snder)
    : Message(msg_snder) {}

void EmailMessage::send() { msg_snder_->sendMessage(); }

void TextMessageSender::sendMessage() {
    std::cout << "send text message..." << std::endl;
}

void EmailMessageSender::sendMessage() {
    std::cout << "send email message..." << std::endl;
}
