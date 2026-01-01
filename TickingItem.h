#pragma once

#include <4dm.h>

class TickingItem : public fdm::Item
{
public:
	bool tickEntityItem = true;
	bool tickHandItem = true;
	bool tickInventoryItem = true;
	virtual void update(fdm::World* world, double dt) = 0;
};
