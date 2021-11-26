/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright IBM Corp. 2021 */

#include "inventory.hpp"
#include "platforms/rainier.hpp"

void Rainier0z::enrollWith(PlatformManager& pm)
{
    pm.enrollPlatform("Rainier 1S4U Pass 1", this);
    pm.enrollPlatform("Rainier 4U Pass 1", this);
    pm.enrollPlatform("Rainier 2U Pass 1", this);
}

void Rainier0z::detectFrus(Notifier& notifier, Inventory* inventory)
{
    Nisqually0z nisqually(inventory);
    Ingraham ingraham(inventory, &nisqually);

    /* Cold-plug devices */
    ingraham.plug(notifier);

    /* Hot-plug devices */
    notifier.run();

    /* Clean up the application state but leave the inventory in-tact. */
    ingraham.unplug(notifier, ingraham.UNPLUG_RETAINS_INVENTORY);
}

void Rainier1z::enrollWith(PlatformManager& pm)
{
    pm.enrollPlatform("Rainier 1S4U", this);
    pm.enrollPlatform("Rainier 4U", this);
    pm.enrollPlatform("Rainier 2U", this);
}

void Rainier1z::detectFrus(Notifier& notifier, Inventory* inventory)
{
    PublishWhenPresentInventoryDecorator decoratedInventory(inventory);
    Nisqually1z nisqually(&decoratedInventory);
    Ingraham ingraham(&decoratedInventory, &nisqually);

    /* Cold-plug devices */
    ingraham.plug(notifier);

    /* Hot-plug devices */
    notifier.run();

    /* Clean up the application state but leave the inventory in-tact. */
    ingraham.unplug(notifier, ingraham.UNPLUG_RETAINS_INVENTORY);
}
