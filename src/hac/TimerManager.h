#pragma once

void enableTimers(bool silent);
void disableTimers();
void resetTimer();
void beginTimerAuto();
void beginTimerUser();
void endTimer(bool silent);
void timerBroadcast(bool broadcast);