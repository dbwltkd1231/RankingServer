#pragma once
#include<memory>

#define NOMINMAX
#include <winsock2.h>
#include<MSWSock.h>


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

    // buffer을 Overlapped 구조체가 직접 가지고 있을 경우 초기화 관리가 어려운것같아 Client객체에서 관리하도록 시도하였습니다.
    struct CustomOverlapped :OVERLAPPED
    {
        WSABUF mWsabuf[2];
        SOCKET* mSocketPtr;
        OperationType mOperationType;

        CustomOverlapped()
        {
            mSocketPtr = nullptr;
            mWsabuf[0].buf = nullptr;
            mWsabuf[0].len = 0;
            mWsabuf[1].buf = nullptr;
            mWsabuf[1].len = 0;
            mOperationType = OperationType::OP_DEFAULT;
			this->hEvent = NULL;
        }

        ~CustomOverlapped()
        {
            mSocketPtr = nullptr;
            mWsabuf[0].buf = nullptr;
            mWsabuf[0].len = 0;
            mWsabuf[1].buf = nullptr;
            mWsabuf[1].len = 0;
            mOperationType = OperationType::OP_DEFAULT;
            this->hEvent = NULL;
        }

        // 복사 생성자
        CustomOverlapped(const CustomOverlapped& other)
        {
			this->hEvent = other.hEvent;

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

            mOperationType = other.mOperationType;
        }

        void SetHeader(char* headerBuffer, ULONG headerLen)
        {
            mWsabuf[0].buf = headerBuffer;
            mWsabuf[0].len = headerLen;
        }

        void SetBody(char* bodyBuffer, ULONG bodyLen)
        {
            mWsabuf[1].buf = bodyBuffer;
            mWsabuf[1].len = bodyLen;
        }

		void SetOperationType(OperationType operationType)
		{
			mOperationType = operationType;
		}

        void Clear()
        {
            mWsabuf[0].buf = nullptr;
            mWsabuf[0].len = 0;
            mWsabuf[1].buf = nullptr;
            mWsabuf[1].len = 0;
            mOperationType = OperationType::OP_DEFAULT;
            this->hEvent = NULL;
        }
    };
}