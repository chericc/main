#include "messager.hpp"

int main()
{
    auto text_msger = std::make_shared<TextMessageSender>();
    auto text_msg = std::make_shared<TextMessage>(text_msger);

    text_msg->send();

    auto email_msger = std::make_shared<EmailMessageSender>();
    auto email_msg = std::make_shared<EmailMessage>(email_msger);

    email_msg->send();

    return 0;
}