#pragma once

#include <stdint.h>

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define htons(n) (n)
#define ntohs(n) (n)
#else
#define htons(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#define ntohs(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#endif

// big endian uint16_t handling
class be_uint16_t {
public:
    be_uint16_t() : be_val_(0) {
    }
    // Transparently cast from uint16_t
    be_uint16_t(const uint16_t &val) : be_val_(htons(val)) {
    }
    // Transparently cast to uint16_t
    operator uint16_t() const {
            return ntohs(be_val_);
    }
private:
    uint16_t be_val_;
} __attribute__((packed));

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define htonl(n) (n)
#define ntohl(n) (n)
#else
#define htonl(n) (((((unsigned long)(n) & 0xFF)) << 24) | (((unsigned long)(n) & 0xFF00) << 8) | (((unsigned long)(n) & 0xFF0000) >> 8) | (((unsigned long)(n) & 0xFF000000) >> 24))
#define ntohl(n) htonl(n)
#endif

// big endian uint32_t handling
class be_uint32_t {
public:
    be_uint32_t() : be_val_(0) {
    }
    // Transparently cast from uint16_t
    be_uint32_t(const uint32_t &val) : be_val_(htonl(val)) {
    }
    // Transparently cast to uint16_t
    operator uint32_t() const {
            return ntohl(be_val_);
    }
private:
    uint32_t be_val_;
} __attribute__((packed));


typedef struct __attribute__((packed)) {
    uint8_t ihl : 4;
    uint8_t version : 4;
    uint8_t ecn : 2;
    uint8_t dscp : 6;
    be_uint16_t length;
    be_uint16_t identification;
    uint8_t fragmentOffsetLow;
    uint8_t fragmentOffsetHigh : 5;
    uint8_t flags : 3;
    uint8_t ttl;
    uint8_t protocol;
    be_uint16_t headerChecksum;
    be_uint32_t srcIp;
    be_uint32_t dstIp;
    uint8_t *payload;
} ipHdr_t;

#define IPV4_PROTOCOL_UDP 0x11

typedef struct __attribute__((packed)) {
    be_uint16_t srcPort;
    be_uint16_t dstPort;
    be_uint16_t length;
    be_uint16_t checksum;
    uint8_t *payload;
} udpHdr_t;

#define udpHdrLen 8