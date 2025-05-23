#pragma once


#define NOMINMAX
#include <winsock2.h>
#include<MSWSock.h>
#include <windows.h>

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
        uint32_t socket_id;
        uint32_t body_size;
        uint32_t contents_type;

        MessageHeader(uint32_t socketID, uint32_t bodySize, uint32_t contentsType) : socket_id(socketID), body_size(bodySize), contents_type(contentsType)
        {

        }

        MessageHeader(const MessageHeader& other) : socket_id(other.socket_id), body_size(other.body_size), contents_type(other.contents_type)
        {
        }
    };


    // buffer을 Overlapped 구조체가 직접 가지고 있을 경우 초기화 관리가 어려운것같아 Client객체에서 관리하도록 시도하였습니다.
    struct CustomOverlapped :OVERLAPPED
    {
        WSABUF wsabuf[2];
        MessageHeader* header;
        OperationType mOperationType;

        CustomOverlapped()
        {
            header = nullptr;
            wsabuf[0].buf = nullptr;
            wsabuf[0].len = 0;
            wsabuf[1].buf = nullptr;
            wsabuf[1].len = 0;
            mOperationType = OperationType::OP_DEFAULT;
        }

        ~CustomOverlapped()
        {
            header = nullptr;
            wsabuf[0].buf = nullptr;
            wsabuf[0].len = 0;
            wsabuf[1].buf = nullptr;
            wsabuf[1].len = 0;
            mOperationType = OperationType::OP_DEFAULT;
        }

        // 복사 생성자
        CustomOverlapped(const CustomOverlapped& other)
        {
            if (other.header)
            {
                header = other.header;
                wsabuf[0].buf = other.wsabuf[0].buf;
                wsabuf[0].len = other.wsabuf[0].len;
            }
            else
            {
                header = nullptr;
                wsabuf[0].buf = nullptr;
                wsabuf[0].len = 0;
            }

            if (other.wsabuf[1].buf != nullptr)
            {
                wsabuf[1].buf = other.wsabuf[1].buf;
                wsabuf[1].len = other.wsabuf[1].len;
            }
            else
            {
                wsabuf[1].buf = nullptr;
                wsabuf[1].len = 0;
            }

            mOperationType = other.mOperationType;
        }

        void Initialize(char* headerBuffer, char* bodyBuffer, ULONG headerLen, ULONG bodyLen)
        {
            wsabuf[0].buf = headerBuffer;
            wsabuf[0].len = headerLen;
            wsabuf[1].buf = bodyBuffer;
            wsabuf[1].len = bodyLen;
        }

		void SetOperationType(OperationType operationType)
		{
			mOperationType = operationType;
		}

        void Clear()
        {
            header = nullptr;
            wsabuf[0].buf = nullptr;
            wsabuf[0].len = 0;
            wsabuf[1].buf = nullptr;
            wsabuf[1].len = 0;
            mOperationType = OperationType::OP_DEFAULT;
        }
    };
}