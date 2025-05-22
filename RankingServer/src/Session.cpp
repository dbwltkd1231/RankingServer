#pragma once
#include "Session.h"

namespace Network
{
	Session::Session()
	{
		
	}

	Session::~Session()
	{
	
	}

	void Session::Initialize()
	{

		mActive = true;
	}

	void Session::Deinitialize()
	{
		mActive = false;

	}

	void Session::Process()
	{
		
	}
}