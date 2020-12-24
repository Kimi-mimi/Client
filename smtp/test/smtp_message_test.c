//
// Created by Dmitry Gorin on 20.12.2020.
//

#include <CUnit/Basic.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../smtp_message.h"
#include "../../bytes/string.h"
#include "smtp_message_test.h"

static SMTPMessage *message = NULL;

#define testDirName             "test_dir"

#define filename1               testDirName "/m1.txt"
#define file1XFrom              "X-KIMI-From: a@a.com"
#define file1XTo                "X-KIMI-To: b@b.com"
#define file1From               "From: a@a.com"
#define file1To                 "To: b@b.com"
#define file1Subject            "Subject: Hello"
#define file1Text               "Hello, world!\n"
static const char *file1Content = file1XFrom "\n"
        file1XTo "\n"
        file1From "\n"
        file1To "\n"
        file1Subject "\n\n"
        file1Text "\n.\n";

#define filename2               testDirName "/m2.txt"
#define file2XFrom              "X-KIMI-From: b@b.com"
#define file2XTo1               "X-KIMI-To: a@a.com"
#define file2XTo2               "X-KIMI-To: c@a.com"
#define file2From               "From: b@b.com"
#define file2To                 "To: a@a.com"
#define file2Subject            "Subject: Hello"
#define file2Text               "Hello, world!\n"
static const char *file2Content = file2XFrom "\n"
        file2XTo1 "\n"
        file2XTo2 "\n"
        file2From "\n"
        file2To "\n"
        file2Subject "\n\n"
        file2Text "\n.\n";

static int fillFileWithContent(const char *filename, const char* content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        return -1;
    }

    fprintf(f, "%s", content);

    fclose(f);
    return 0;
}

static int fillSmtpMessage() {
    String *from, *to, *subj, *msg;
    from = to = subj = msg = NULL;

    message = smtpMessageInit();
    if (!message) {
        return -1;
    }

    if ((from = stringInitFromStringBuf(file1XFrom)) == NULL ||
            (to = stringInitFromStringBuf(file1XTo)) == NULL ||
            (subj = stringInitFromStringBuf(file1Subject)) == NULL ||
            (msg = stringInitFromStringBuf(file1Content)) == NULL ||
            (message->recipients = calloc(1, sizeof(String*))) == NULL ||
            stringConcat(message->from, from) < 0 ||
            (message->recipients[0] = stringInitCopy(to)) == NULL ||
            stringConcat(message->data, msg) < 0) {
        stringDeinit(from);
        stringDeinit(to);
        stringDeinit(subj);
        stringDeinit(msg);
        smtpMessageDeinit(message);
        return -1;
    }

    message->recipientsCount = 1;
    stringDeinit(from);
    stringDeinit(to);
    stringDeinit(subj);
    stringDeinit(msg);
    return 0;
}

int initSuiteSmtpMessage(void) {
    struct stat st = {0};
    if (stat(testDirName, &st) < 0) {
        mkdir(testDirName, 0700);
    }

    if (fillFileWithContent(filename1, file1Content) < 0 ||
        fillFileWithContent(filename2, file2Content) < 0) {
        return -1;
    }

    if (fillSmtpMessage() < 0) {
        return -1;
    }

    return 0;
}

int cleanupSuiteSmtpMessage(void) {
    rmdir(testDirName);
    smtpMessageDeinit(message);
    message = NULL;
    return 0;
}

void testInitFromFile(void) {
    SMTPMessage *testMessage = smtpMessageInitFromFile(filename1);
    CU_ASSERT_PTR_NOT_NULL_FATAL(testMessage)

    CU_ASSERT_EQUAL(testMessage->recipientsCount, 1)

    String *fromHeader = smtpMessageGetFromHeader(testMessage);
    CU_ASSERT_PTR_NOT_NULL_FATAL(fromHeader)
    CU_ASSERT_EQUAL(strcmp(fromHeader->buf, file1XFrom), 0)
    stringDeinit(fromHeader);

    CU_ASSERT_EQUAL(strcmp(testMessage->data->buf, file1Content), 0)

    smtpMessageDeinit(testMessage);
}

void testInitFromDir(void) {
    int messagesCount = 0;
    SMTPMessage **testMessages = smtpMessageInitFromDir(testDirName, &messagesCount);
    CU_ASSERT_PTR_NOT_NULL_FATAL(testMessages)
    CU_ASSERT_EQUAL_FATAL(messagesCount, 2)

    for (int i = 0; i < messagesCount; i++) {
        smtpMessageDeinit(testMessages[i]);
    }
    if (testMessages) {
        free(testMessages);
    }
}

void testAddRecipient(void) {
    int addResult = 0;
    String *recipient = stringInitFromStringBuf(file1XTo);
    CU_ASSERT_PTR_NOT_NULL_FATAL(recipient)

    addResult = smtpMessageAddRecipient(message, recipient);
    CU_ASSERT_EQUAL_FATAL(addResult, 2)
    CU_ASSERT_EQUAL(message->recipientsCount, 2)

    stringDeinit(recipient);
}

void testGetDomainFromEmailAddress(void) {
    String *emailAddress = stringInitFromStringBuf("a@a.com");
    String *domainPerfect = stringInitFromStringBuf("a.com");
    CU_ASSERT_PTR_NOT_NULL_FATAL(emailAddress)
    CU_ASSERT_PTR_NOT_NULL_FATAL(domainPerfect)

    String *domain = getDomainFromEmailAddress(emailAddress);
    CU_ASSERT_PTR_NOT_NULL(domain)
    CU_ASSERT_EQUAL(stringEqualsTo(domainPerfect, domain), 1)

    stringDeinit(emailAddress);
    stringDeinit(domainPerfect);
    stringDeinit(domain);
}

void testGetRecipientsDomainsDistinct(void) {
    String **domains = NULL;
    size_t domainsCount = 0;

    if (message->recipientsCount < 2) {
        String *recipient = stringInitFromStringBuf(file1XTo);
        CU_ASSERT_PTR_NOT_NULL_FATAL(recipient)
        CU_ASSERT_EQUAL_FATAL(smtpMessageAddRecipient(message, recipient), 0)
        stringDeinit(recipient);
    }

    domains = smtpMessageGetRecipientsDomainsDistinct(message, &domainsCount);
    CU_ASSERT_PTR_NOT_NULL_FATAL(domains)
    CU_ASSERT_EQUAL(domainsCount, 1)

    for (int i = 0; i < domainsCount; i++) {
        stringDeinit(domains[i]);
    }
    if (domains) {
        free(domains);
    }
}

int fillSuiteWithTestsSmtpMessage(CU_pSuite suite) {
    if (!CU_add_test(suite, "Test SMTP message init from file", testInitFromFile) ||
        !CU_add_test(suite, "Test SMTP message init from dir", testInitFromDir) ||
        !CU_add_test(suite, "Test SMTP message add recipient", testAddRecipient) ||
        !CU_add_test(suite, "Test SMTP message get domain from email address", testGetDomainFromEmailAddress) ||
        !CU_add_test(suite, "Test SMTP message get distinct domains from recipients", testGetRecipientsDomainsDistinct)) {
        return CU_get_error();
    }
    return CUE_SUCCESS;
}
