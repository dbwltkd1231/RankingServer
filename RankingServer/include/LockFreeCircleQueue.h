#pragma once

#include<iostream>
#include<atomic>

namespace Utility
{
	template<typename T>
	class LockFreeCircleQueue
	{
	private:
		int queueMaxSize;// 실질적으로는 queueMaxSize-1만큼 넣을수있다. 원형큐는 하나의 공간을 비워둠으로써 full과 empty상태를 구분하기 때문.
		std::atomic<int> inputIndex;
		std::atomic<int> outputIndex;
		T* buffer;
	public:
		LockFreeCircleQueue<T>()
		{
			inputIndex = 0;
			outputIndex = 0;
		}

		~LockFreeCircleQueue<T>()
		{
			delete[] buffer;
		}

		void Construct(int queueMaxSize)
		{
			this->queueMaxSize = queueMaxSize;
			buffer = new T[queueMaxSize];
		}

		//여기서 &&은 rvalue reference를 의미한다.
		// rvalue reference는 임시 객체를 참조하는데 사용되며, std::move()와 함께 사용하여 객체의 소유권을 이동을 목적으로 할때 사용된다.
		//최적화 이동을 위해 중요한 역할을한다.
		bool push(T&& data)
		{
			int currentInputIndex = inputIndex.load(std::memory_order_acquire);
			int nextIndex = (currentInputIndex + 1) % queueMaxSize;

			if (nextIndex == outputIndex.load(std::memory_order_acquire))
			{
				std::cout << "Queue is full" << std::endl;
				return false;
			}

			buffer[currentInputIndex] = std::move(data);
			inputIndex.store(nextIndex, std::memory_order_release);// 업데이트 후 release
			return true;
		}

		T pop()
		{
			int currentOutputIndex = outputIndex.load(std::memory_order_acquire); // 최신 outputIndex를 가져와서 비교한다.

			if (currentOutputIndex == inputIndex.load(std::memory_order_acquire))//데이터를 읽는 load 작업에서 사용하여 성능 최적화.
			{
				std::cout << "Queue is empty" << std::endl;
				return T();
			}

			T data = std::move(buffer[currentOutputIndex]);
			outputIndex.store((currentOutputIndex + 1) % queueMaxSize, std::memory_order_release);// 데이터를 완전히 기록한 뒤 인덱스를 업데이트할 때 사용
			return data;
		}

		bool empty()
		{
			return inputIndex.load(std::memory_order_acquire) == outputIndex.load(std::memory_order_acquire);
		}

		//TODO Atomic	//size()와 capacity()는 atomic을 사용하지 않음. 단순히 인덱스 차이로 계산하기 때문. ???
		int size()
		{
			auto input = inputIndex.load(std::memory_order_acquire);
			auto output = outputIndex.load(std::memory_order_acquire);

			if (input >= output)
				return input - output;
			else
				return queueMaxSize - outputIndex + input;
		}

		int capacity()
		{
			return queueMaxSize;
		}

		void clear()
		{
			inputIndex.store(0, std::memory_order_release);
			outputIndex.store(0, std::memory_order_release);
		}

		void print()
		{
			std::cout << "Input Index: " << inputIndex << ", Output Index: " << outputIndex << std::endl;
			for (int i = 0; i < size(); i++)
			{
				std::cout << buffer[(outputIndex + i) % queueMaxSize] << " ";
			}
			std::cout << std::endl;
		}

		T Front()
		{
			return buffer[outputIndex.load(std::memory_order_acquire)];
		}
	};
}
