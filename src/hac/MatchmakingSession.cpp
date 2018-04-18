#include "MatchmakingSession.h"

namespace Matchmaking {

DWORD Session::run() {
	while(!exit) {
		WaitForSingleObject(exit_sem_, INFINITE);
		process_queue();
	}

	ReleaseSemaphore(exit_sem_, 1, NULL);
	return 0;
}

void Session::wait() {
	WaitForSingleObject(exit_sem_, INFINITE);
}

void Session::process_queue() {
	std::shared_ptr<Event> event;

	while(!events.empty()) {
		if(!events.try_pop(event)) {
			return;
		}

		switch(event->type) {
			case THREAD_EXIT:
				exit_ = true;
				continue;
		}
	}
}

Session::~Session() {
	dispatcher_->unsubscribe(this);
	CloseHandle(exit_sem_);
}


} // Matchmaking