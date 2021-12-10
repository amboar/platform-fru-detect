/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright IBM Corp. 2021 */
#include "inventory.hpp"

#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/bus/match.hpp>
#include <sdbusplus/message.hpp>

PHOSPHOR_LOG2_USING;

static constexpr auto INVENTORY_BUS_NAME =
    "xyz.openbmc_project.Inventory.Manager";

// https://github.com/openbmc/phosphor-dbus-interfaces/blob/08baf48ad5f15774d393fbbf4e9479a0ef3e82d0/yaml/xyz/openbmc_project/Inventory/Manager.interface.yaml
static constexpr auto INVENTORY_MANAGER_IFACE =
    "xyz.openbmc_project.Inventory.Manager";
static constexpr auto INVENTORY_MANAGER_OBJECT =
    "/xyz/openbmc_project/inventory";

// https://dbus.freedesktop.org/doc/dbus-specification.html#standard-interfaces-properties
static constexpr auto DBUS_PROPERTY_IFACE = "org.freedesktop.DBus.Properties";

static constexpr auto INVENTORY_DECORATOR_ASSET_IFACE =
    "xyz.openbmc_project.Inventory.Decorator.Asset";

using namespace inventory;
using namespace dbus;

std::weak_ptr<PropertiesChangedListener>
    InventoryManager::addPropertiesChangedListener(
        const std::string& path, const std::string& interface,
        std::function<void(PropertiesChanged&&)> callback)
{
    auto pcl = sharedPropertiesChangedListener(dbus, path, interface, callback);
    return *(listeners.insert(pcl).first);
}

void InventoryManager::removePropertiesChangedListener(
    std::weak_ptr<PropertiesChangedListener> listener)
{
    std::shared_ptr<PropertiesChangedListener> shared = listener.lock();
    if (!listeners.contains(shared))
    {
        debug("Cannot remove unrecognised PropertiesChangedListener");
        return;
    }

    listeners.erase(shared);
}

void InventoryManager::updateObject(const std::string& path,
                                    const ObjectType& updates)
{
    auto call =
        dbus.new_method_call(INVENTORY_BUS_NAME, INVENTORY_MANAGER_OBJECT,
                             INVENTORY_MANAGER_IFACE, "Notify");

    std::map<sdbusplus::message::object_path, ObjectType> inventoryUpdate = {
        {path, updates},
    };

    call.append(inventoryUpdate);
    dbus.call(call);
}

void InventoryManager::add(const std::string& path, const interfaces::Interface iface)
{
    ObjectType updates;

    iface.populateObject(updates);

    updateObject(path, updates);
}

void InventoryManager::remove(const std::string& path, const interfaces::Interface iface)
{
    ObjectType updates;

    iface.depopulateObject(updates);

    updateObject(path, updates);
}

void InventoryManager::markPresent(const std::string& path)
{
    std::string absolute = std::string("/xyz/openbmc_project/inventory") + path;

    auto call = dbus.new_method_call(INVENTORY_BUS_NAME, absolute.c_str(),
                                     DBUS_PROPERTY_IFACE, "Set");

    call.append(INVENTORY_ITEM_IFACE, "Present", std::variant<bool>(true));
    dbus.call(call);
}

void InventoryManager::markAbsent(const std::string& path)
{
    std::string absolute = std::string("/xyz/openbmc_project/inventory") + path;

    auto call = dbus.new_method_call(INVENTORY_BUS_NAME, absolute.c_str(),
                                     DBUS_PROPERTY_IFACE, "Set");

    call.append(INVENTORY_ITEM_IFACE, "Present", std::variant<bool>(false));
    dbus.call(call);
}

bool InventoryManager::isPresent(const std::string& path)
{
    std::string absolute = std::string("/xyz/openbmc_project/inventory") + path;

    auto call = dbus.new_method_call(INVENTORY_BUS_NAME, absolute.c_str(),
                                     DBUS_PROPERTY_IFACE, "Get");

    call.append(INVENTORY_ITEM_IFACE, "Present");

    try
    {
        std::variant<bool> present;
        auto reply = dbus.call(call);
        reply.read(present);

        return std::get<bool>(present);
    }
    catch (const sdbusplus::exception::exception& ex)
    {
        /* TODO: Define an exception for inventory operations? */
        info("Failed to determine device presence at path {INVENTORY_PATH}",
             "INVENTORY_PATH", absolute);
        debug(
            "Inventory lookup for device at path {INVENTORY_PATH} resulted in an exception: {EXCEPTION}",
            "INVENTORY_PATH", absolute, "EXCEPTION", ex);
    }

    return false;
}

bool InventoryManager::isModel(const std::string& path,
                               const std::string& model)
{
    std::string absolute = std::string("/xyz/openbmc_project/inventory") + path;

    auto call = dbus.new_method_call(INVENTORY_BUS_NAME, absolute.c_str(),
                                     DBUS_PROPERTY_IFACE, "Get");

    call.append(INVENTORY_DECORATOR_ASSET_IFACE, "Model");

    try
    {
        std::variant<std::string> found;
        auto reply = dbus.call(call);
        reply.read(found);

        return std::get<std::string>(found) == model;
    }
    catch (const sdbusplus::exception::exception& ex)
    {
        /* TODO: Define an exception for inventory operations? */
        info("Failed to determine device presence at path {INVENTORY_PATH}",
             "INVENTORY_PATH", absolute);
        debug(
            "Inventory lookup for device at path {INVENTORY_PATH} resulted in an exception: {EXCEPTION}",
            "INVENTORY_PATH", absolute, "EXCEPTION", ex);
    }

    return false;
}
