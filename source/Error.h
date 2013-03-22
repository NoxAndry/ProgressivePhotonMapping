#pragma once

#include <string>
#include <Windows.h>

class Error
{
public:
	Error(char * c)
		: message(c)
	{
	}

	Error(const std::string& s)
		: message(s)
	{
	}

	void HandleError() const
	{
		const std::string s = "Error: \"" + message + "\". Application aborted.";
		MessageBox(0, s.c_str(), 0, 0);
	}

	static void HandleDefaultError()
	{
		MessageBox(0, "Unknown Error", 0, 0);
	}

private:
	std::string message;
	Error operator=(const Error& err);
};