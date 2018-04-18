#pragma once

#include "UIControl.h"
#include <string>
#include <cstdint>
#include <Windows.h>

class OverlayPatcher;

class DisplayUpdater {
	public:
		DisplayUpdater();
		DisplayUpdater(const std::string& map);
		~DisplayUpdater();
		void showMessage(const std::string& message) const;
		void showMessage(const wchar_t* message) const;
		void updateProgress(float done, float remaining, int speed);
		void begin() const;
		bool restore;
		void unpatch();
		static void notify();

	private:
		static const wchar_t* format;
		static DisplayUpdater* instance;
		std::string map;
		void* location;
		DisplayString original;
		WCHAR* message;
		void getLocation();
		mutable std::uint32_t oldOverlay;
		static CRITICAL_SECTION csection;
		//CONTEXT context;
		OverlayPatcher* overlayPatch;
		void initialise();
		DisplayUpdater(const DisplayUpdater&);
};