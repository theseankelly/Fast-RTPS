
#ifndef RTCP_HEADER_H
#define RTCP_HEADER_H

#include <fastrtps/topic/TopicDataType.h>
#include <fastrtps/rtps/common/Types.h>
#include <cstring>
#include <fastcdr/FastCdr.h>
#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

#define TCPHEADER_SIZE 14

// TCP Header structs and enums.
struct TCPHeader
{
    char rtcp[4];
    uint32_t length;
    uint32_t crc;
    uint16_t logical_port;

    TCPHeader()
        : length(sizeof(TCPHeader))
        , crc(0)
        , logical_port(0)
    {
        // There isn't a explicit constructor because VS2013 doesn't support it.
        rtcp[0] = 'R';
        rtcp[1] = 'T';
        rtcp[2] = 'C';
        rtcp[3] = 'P';
    }

    const octet* address() const
    {
        return reinterpret_cast<const octet*>(this);
    }

    octet* address()
    {
        return (octet*)this;
    }

    /*!
     * @brief This function returns the maximum serialized size of an object
     * depending on the buffer alignment.
     * @return Maximum serialized size.
     */
    static inline size_t size()
    {
        return TCPHEADER_SIZE;
    }
};

union TCPTransactionId
{
    uint32_t ints[3];
    octet octets[12];

    TCPTransactionId()
    {
        memset(ints, 0, 3 * sizeof(uint32_t));
    }

    TCPTransactionId(const TCPTransactionId& t)
    {
        memcpy(ints, t.ints, 3 * sizeof(uint32_t));
    }

    TCPTransactionId& operator++()
    {
        if (ints[0] == 0xffffffff)
        {
            if (ints[1] == 0xffffffff)
            {
                if (ints[2] == 0xffffffff)
                {
                    memset(ints, 0, 3 * sizeof(uint32_t));
                }
                else
                {
                    ints[2] += 1;
                }
            }
            else
            {
                ints[1] += 1;
            }
        }
        else
        {
            ints[0] += 1;
        }
        return *this;
    }

    TCPTransactionId operator++(int)
    {
        TCPTransactionId prev = *this;
        ++(*this);
        return prev;
    }


    TCPTransactionId& operator=(const TCPTransactionId& t)
    {
        memcpy(ints, t.ints, 3 * sizeof(uint32_t));
        return *this;
    }

    TCPTransactionId& operator=(const octet* id)
    {
        memcpy(octets, id, 12 * sizeof(octet));
        return *this;
    }

    TCPTransactionId& operator=(const char* id)
    {
        memcpy(octets, id, 12 * sizeof(octet));
        return *this;
    }

    TCPTransactionId& operator=(const uint32_t* id)
    {
        memcpy(ints, id, 3 * sizeof(uint32_t));
        return *this;
    }

    TCPTransactionId& operator=(uint32_t id)
    {
        ints[0] = id;
        ints[1] = 0;
        ints[2] = 0;
        return *this;
    }

    TCPTransactionId& operator=(uint64_t id)
    {
        memset(ints, 0, sizeof(uint32_t) * 3);
        memcpy(ints, &id, sizeof(uint64_t));
        return *this;
    }

    bool operator==(const TCPTransactionId& t) const
    {
        return memcmp(ints, t.ints, 3 * sizeof(uint32_t)) == 0;
    }

    bool operator<(const TCPTransactionId& t) const
    {
        return memcmp(ints, t.ints, 3 * sizeof(uint32_t)) < 0;
    }
};

inline std::ostream& operator<<(std::ostream& output,const TCPTransactionId& t)
{
    bool printed = false; // Don't skip cases like 99 0 34
    for (int i = 2; i >= 0; --i)
    {
        if (printed || i == 0 || t.ints[i] > 0)
        {
            output << t.ints[i];
            printed = true;
        }
    }
    return output;
}

enum TCPCPMKind : octet
{
    BIND_CONNECTION_REQUEST =           0xD1,
    BIND_CONNECTION_RESPONSE =          0xE1,
    OPEN_LOGICAL_PORT_REQUEST =         0xD2,
    OPEN_LOGICAL_PORT_RESPONSE =        0xE2,
    CHECK_LOGICAL_PORT_REQUEST =        0xD3,
    CHECK_LOGICAL_PORT_RESPONSE =       0xE3,
    KEEP_ALIVE_REQUEST =                0xD4,
    KEEP_ALIVE_RESPONSE =               0xE4,
    LOGICAL_PORT_IS_CLOSED_REQUEST =    0xD5,
    UNBIND_CONNECTION_REQUEST =         0xD6
};

class TCPControlMsgHeader
{
    TCPCPMKind kind_; // 1 byte
    octet flags_; // 1 byte
    uint16_t length_; // 2 bytes
    TCPTransactionId transaction_id_; // 12 bytes

public:
    TCPControlMsgHeader()
    {
        kind_ = static_cast<TCPCPMKind>(0x00);
        flags_ = static_cast<octet>(0x00);
        length_ = 0;
    }

    void kind(TCPCPMKind kind)
    {
        kind_ = kind;
    }

    TCPCPMKind kind() const
    {
        return kind_;
    }

    TCPCPMKind& kind()
    {
        return kind_;
    }

    void length(uint16_t length)
    {
        length_ = length;
    }

    uint16_t length() const
    {
        return length_;
    }

    uint16_t& length()
    {
        return length_;
    }

    void transaction_id(TCPTransactionId transaction_id)
    {
        transaction_id_ = transaction_id;
    }

    TCPTransactionId transaction_id() const
    {
        return transaction_id_;
    }

    TCPTransactionId& transaction_id()
    {
        return transaction_id_;
    }

    void flags(
            bool endianess,
            bool payload,
            bool requires_response)
    {
        //TODO: Optimize receiving a Endianness_t
        octet e = (endianess) ? BIT(1) : 0x00;
        octet p = (payload) ? BIT(2) : 0x00;
        octet r = (requires_response) ? BIT(3) : 0x00;
        flags_ = e | p | r;
    }

    void endianess(Endianness_t endianess)
    {
        // Endianess flag has inverse logic than Endianness_t :-/
        if (endianess == Endianness_t::BIGEND)
        {
            flags_ &= 0xFE;
        }
        else
        {
            flags_ |= BIT(1);
        }
    }

    void payload(bool payload)
    {
        if (payload)
        {
            flags_ |= BIT(2);
        }
        else
        {
            flags_ &= 0xFD;
        }
    }

    void requires_response(bool requires_response)
    {
        if (requires_response)
        {
            flags_ |= BIT(3);
        }
        else
        {
            flags_ &= 0xFB;
        }
    }

    bool endianess()
    {
        return (flags_ & BIT(1)) != 0;
    }

    bool payload()
    {
        return (flags_ & BIT(2)) != 0;
    }

    bool requires_response()
    {
        return (flags_ & BIT(3)) != 0;
    }

    static inline size_t size()
    {
        return 16;
    }
};


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // RTCP_HEADER_H
