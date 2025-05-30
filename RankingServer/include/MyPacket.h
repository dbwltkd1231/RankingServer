#pragma once
#include<memory>

#define NOMINMAX
#include <winsock2.h>
#include<MSWSock.h>

#define BUFFER_SIZE 1024

namespace Network
{
    enum OperationType
    {
        OP_DEFAULT = 0,
        OP_ACCEPT = 1,
        OP_RECV = 2,
        OP_SEND = 3,
    };

    struct MessageHeader
    {
        uint32_t mSocketId;
        uint32_t mBodySize;
        uint32_t mContentsType;

        MessageHeader(uint32_t socketID, uint32_t bodySize, uint32_t contentsType) : mSocketId(socketID), mBodySize(bodySize), mContentsType(contentsType)
        {

        }

        MessageHeader(const MessageHeader& other) : mSocketId(other.mSocketId), mBodySize(other.mBodySize), mContentsType(other.mContentsType)
        {
        }
    };

    struct CustomOverlapped :OVERLAPPED
    {
        WSABUF mWsabuf[2];
        SOCKET* mSocketPtr;
        OperationType mOperationType;

        CustomOverlapped()
        {
            //mWsabuf[0].buf = nullptr;
            mWsabuf[0].buf = new char[sizeof(MessageHeader)];
            mWsabuf[0].len = sizeof(MessageHeader);

            // mWsabuf[1].buf = nullptr;
            mWsabuf[1].buf = new char[BUFFER_SIZE];
            mWsabuf[1].len = BUFFER_SIZE;

            mSocketPtr = nullptr;
            mOperationType = OperationType::OP_DEFAULT;
            this->hEvent = NULL;
        }

        ~CustomOverlapped()
        {
            mSocketPtr = nullptr;

            //mWsabuf[0].buf = nullptr;
            delete[] mWsabuf[0].buf;
            mWsabuf[0].len = 0;

            //mWsabuf[1].buf = nullptr;
            delete[] mWsabuf[1].buf;
            mWsabuf[1].len = 0;

            mOperationType = OperationType::OP_DEFAULT;
            this->hEvent = NULL;
        }

        // 복사 생성자
        CustomOverlapped(const CustomOverlapped& other)
        {
            if (other.mWsabuf[0].len > 0)
            {
                mWsabuf[0].buf = other.mWsabuf[0].buf;
                mWsabuf[0].len = other.mWsabuf[0].len;
            }
            else
            {
                mWsabuf[0].buf = nullptr;
                mWsabuf[0].len = 0;
            }

            if (other.mWsabuf[1].len > 0)
            {
                mWsabuf[1].buf = other.mWsabuf[1].buf;
                mWsabuf[1].len = other.mWsabuf[1].len;
            }
            else
            {
                mWsabuf[1].buf = nullptr;
                mWsabuf[1].len = 0;
            }

            this->hEvent = other.hEvent;
            mOperationType = other.mOperationType;
        }

        void SetHeader(const MessageHeader& headerData)
        {
            memset(mWsabuf[0].buf, 0, mWsabuf[0].len);

            auto header = new MessageHeader(headerData); // 동적으로 할당
            mWsabuf[0].buf = reinterpret_cast<char*>(header);
            mWsabuf[0].len = sizeof(MessageHeader);
        }

        void SetBody(char* bodyBuffer, ULONG bodyLen)
        {
            memset(mWsabuf[1].buf, 0, bodyLen);

            if (bodyLen > BUFFER_SIZE)
            {
                std::cerr << "Buffer overflow 위험! bodyLen이 너무 큼!" << std::endl;
                return;
            }

            memcpy(mWsabuf[1].buf, bodyBuffer, bodyLen);
            mWsabuf[1].len = bodyLen;

        }

		void SetOperationType(OperationType operationType)
		{
			mOperationType = operationType;
		}

        void Clear()
        {
            memset(mWsabuf[0].buf, 0, sizeof(MessageHeader));
            mWsabuf[0].len = sizeof(MessageHeader);
            memset(mWsabuf[1].buf, 0, mWsabuf[1].len);
            mWsabuf[1].len = BUFFER_SIZE;
            mOperationType = OperationType::OP_DEFAULT;
            this->hEvent = NULL;
        }
    };
}