//
// Created by Dmitry Gorin on 21.11.2020.
//

#ifndef CLIENT_SMTP_COMMAND_H
#define CLIENT_SMTP_COMMAND_H

#include "../bytes/string.h"

#define KIMI_MIMI_DOMAIN_NAME "kimi-mimi.ru"

String *getHELOCommand();
String *getEHLOCommand();
String *getMAILFROMCommand(const String *fromAddress);
String *getRCPTTOCommand(const String *recipientAddress);
String *getDATACommand();
String *getRSETCommand();
String *getQUITCommand();

#endif //CLIENT_SMTP_COMMAND_H
