#pragma once

namespace EngineState {
	/*struct State {
		bool downloading : 1;
		bool queued : 1;
		bool scrim_mode : 1;
		bool multiteam_vehicles : 1;
		bool connection_lock : 1;
		bool halt_download : 1;
		bool leave_queue : 1;
		bool slow_output : 1;
		bool voip_available: 1;
	} state;*/

	enum STATE {
		DEFAULT = 1,
		DOWNLOADING = 2,
		QUEUED = 4,
		SCRIM_MODE = 8,
		MULTITEAM_VEHICLES = 16,
		CONNECTION_LOCK = 32,
		HALT_DOWNLOAD = 64,
		LEAVE_QUEUE = 128,
		SLOW_OUTPUT = 256,
		VOIP_AVAILABLE = 512
	};
	
	void toggleState(STATE state);
	bool checkState(STATE state);
	void clearState(STATE state);
	void setState(STATE state);
};