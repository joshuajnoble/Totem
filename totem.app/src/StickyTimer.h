#pragma once

class StickyTimer
{
private:
	float delayDuration;
	bool isActive = false;
	bool postponed = false;
	float expiration = 0;

public:
	StickyTimer(float timeoutInSeconds);
	bool IsActive() const { return this->isActive; }
	void update(float currentTime);
	void postpone();
};