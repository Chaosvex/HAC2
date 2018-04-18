#pragma once

#include <string>
#include <cstdint>
#include <exception>

class CustomChatException : public std::runtime_error {
public:
	CustomChatException() : std::runtime_error("An unknown exception occured!") { }
	CustomChatException(std::string msg) : std::runtime_error(msg) { };
};

namespace Chat {

extern std::uintptr_t ContinueChat;
void enable(bool chat_hide = false /* total hack */);
void disable();
void chatHandlerStub();
void draw();
void lost();
void reset();
void medalText(const std::string& message);
void resolutionChange();

}