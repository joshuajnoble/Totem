#include "StickyTimer.h"

StickyTimer::StickyTimer(float timeoutInSeconds)
	: delayDuration(timeoutInSeconds)
{
}

void StickyTimer::update(float currentTime)
{
	if (this->postponed || this->expiration == 0)
	{
		this->postponed = false;
		this->expiration = currentTime + this->delayDuration;
	}

	if (currentTime >= this->expiration)
	{
		this->isActive = false;
	}
}

void StickyTimer::postpone()
{
	this->postponed = true;
	this->isActive = false;
}
