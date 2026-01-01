#include "TickingItem.h"

using namespace fdm;

$hook(void, Player, update, World* world, double dt, EntityPlayer* entityPlayer)
{
	original(self, world, dt, entityPlayer);

	for (int i = 0; i < self->hotbar.getSlotCount(); ++i)
	{
		auto& it = self->hotbar.getSlot(i);
		if (auto* item = dynamic_cast<TickingItem*>(it.get()))
		{
			if (i == self->hotbar.selectedIndex ? item->tickHandItem : item->tickInventoryItem)
				item->update(world, dt);
		}
	}
	for (int i = 0; i < self->inventory.getSlotCount(); ++i)
	{
		auto& it = self->inventory.getSlot(i);
		if (auto* item = dynamic_cast<TickingItem*>(it.get()))
		{
			if (item->tickInventoryItem)
				item->update(world, dt);
		}
	}
	if (auto* item = dynamic_cast<TickingItem*>(self->equipment.getSlot(0).get()))
	{
		if (item->tickHandItem)
			item->update(world, dt);
	}
}
$hook(void, EntityItem, update, World* world, double dt)
{
	original(self, world, dt);

	if (auto* item = dynamic_cast<TickingItem*>(self->item.get()))
	{
		if (item->tickEntityItem)
			item->update(world, dt);
	}
}
