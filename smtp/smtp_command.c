//
// Created by Dmitry Gorin on 21.11.2020.
//

#include "../bytes/string.h"
#include "../errors/client_errors.h"
#include "smtp_command.h"



String *getHELOCommand() {
    String *command = NULL;

    command = stringInitFromStringBuf("HELO " KIMI_MIMI_DOMAIN_NAME CRLF);
    if (!command) {
        errPrint();
        return NULL;
    }

    return command;
}

String *getEHLOCommand() {
    String *command = NULL;

    command = stringInitFromStringBuf("EHLO " KIMI_MIMI_DOMAIN_NAME CRLF);
    if (!command) {
        errPrint();
        return NULL;
    }

    return command;
}

String *getMAILFROMCommand(const String *fromAddress) {
    String *command = NULL;
    const String crlfString = CRLF_STRING_INITIALIZER;
    const String openDiamondString = DIAMOND_OPEN_STRING_INITIALIZER;
    const String closeDiamondString = DIAMOND_CLOSE_STRING_INITIALIZER;

    command = stringInitFromStringBuf("MAIL FROM:");
    if (!command) {
        errPrint();
        return NULL;
    }

    if (stringConcat(command, &openDiamondString) < 0 ||
            stringConcat(command, fromAddress) < 0 ||
            stringConcat(command, &closeDiamondString) < 0 ||
            stringConcat(command, &crlfString) < 0) {
        errPrint();
        stringDeinit(command);
        return NULL;
    }

    return command;
}

String *getRCPTTOCommand(const String *recipientAddress) {
    String *command = NULL;
    const String crlfString = CRLF_STRING_INITIALIZER;
    const String openDiamondString = DIAMOND_OPEN_STRING_INITIALIZER;
    const String closeDiamondString = DIAMOND_CLOSE_STRING_INITIALIZER;

    command = stringInitFromStringBuf("RCPT TO:");
    if (!command) {
        errPrint();
        return NULL;
    }

    if (stringConcat(command, &openDiamondString) < 0 ||
            stringConcat(command, recipientAddress) < 0 ||
            stringConcat(command, &closeDiamondString) < 0 ||
            stringConcat(command, &crlfString) < 0) {
        errPrint();
        stringDeinit(command);
        return NULL;
    }

    return command;
}

String *getDATACommand(const String *message) {
    String *command = NULL;
    command = stringInitFromStringBuf("DATA" CRLF);
    if (!command) {
        errPrint();
        return NULL;
    }

    return command;
}

String *getVRFYCommand(const String *username) {
    String *command = NULL;
    const String crlfString = CRLF_STRING_INITIALIZER;

    command = stringInitFromStringBuf("VRFY ");
    if (!command) {
        errPrint();
        return NULL;
    }

    if (stringConcat(command, username) < 0 ||
            stringConcat(command, &crlfString) < 0) {
        errPrint();
        stringDeinit(command);
        return NULL;
    }

    return command;
}

String *getRSETCommand() {
    String *command = NULL;

    command = stringInitFromStringBuf("RSET" CRLF);
    if (!command) {
        errPrint();
        return NULL;
    }

    return command;
}

String *getNOOPCommand() {
    String *command = NULL;

    command = stringInitFromStringBuf("NOOP" CRLF);
    if (!command) {
        errPrint();
        return NULL;
    }

    return command;
}

String *getQUITCommand() {
    String *command = NULL;

    command = stringInitFromStringBuf("QUIT" CRLF);
    if (!command) {
        errPrint();
        return NULL;
    }

    return command;
}
