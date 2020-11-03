//
// Created by Dmitry Gorin on 03.11.2020.
//

#ifndef CLIENT_BYTES_H
#define CLIENT_BYTES_H

#define freeAndNull(buf) if (buf) { free(buf); } buf = NULL

size_t concatBytesWithAllocAndLengths(char**, const char*, size_t, size_t, size_t);

int isBuffersEqual(const char*, const char*, size_t, size_t);
int hasSuffix(const char*, size_t, const char*, size_t);
int hasSubBuffer(const char*, size_t, const char*, size_t);

#endif //CLIENT_BYTES_H
