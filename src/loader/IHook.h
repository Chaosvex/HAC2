#pragma once

class IHook {
public:
	virtual ~IHook() {};
	virtual bool Install() = 0;
	virtual void Uninstall() = 0;
};