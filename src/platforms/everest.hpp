/* SPDX-License-Identifier: Apache-2.0 */
#pragma once

#include "devices/nvme.hpp"
#include "platform.hpp"

#include <gpiod.hpp>

class Basecamp;
class Bellavista;

class BasecampNVMeDrive : public BasicNVMeDrive
{
  public:
    BasecampNVMeDrive(Inventory* inventory, const Basecamp* basecamp,
                      int index);
    ~BasecampNVMeDrive() = default;

    /* Device */
    virtual void plug(Notifier& notifier) override;
    virtual void unplug(Notifier& notifier,
                        int mode = UNPLUG_REMOVES_INVENTORY) override;

    /* FRU */
    virtual std::string getInventoryPath() const override;
    virtual void addToInventory(Inventory* inventory) override;
    virtual void removeFromInventory(Inventory* inventory) override;

  private:
    const Basecamp* basecamp;
};

class Basecamp : public Device, FRU
{
  public:
    explicit Basecamp(Inventory* inventory, const Bellavista* bellavista);
    Basecamp(const Basecamp& other) = delete;
    Basecamp(const Basecamp&& other) = delete;
    ~Basecamp() = default;

    Basecamp& operator=(const Basecamp& other) = delete;
    Basecamp& operator=(const Basecamp&& other) = delete;

    SysfsI2CBus getDriveBus(int index) const;

    /* Device */
    virtual void plug(Notifier& notifier) override;
    virtual void unplug(Notifier& notifier,
                        int mode = Device::UNPLUG_REMOVES_INVENTORY) override;

    /* FRU */
    virtual std::string getInventoryPath() const override;
    virtual void addToInventory(Inventory* inventory) override;
    virtual void removeFromInventory(Inventory* inventory) override;

  private:
    static constexpr const char* drive_metadata_bus =
        "/sys/bus/i2c/devices/i2c-14";
    static constexpr int drive_metadata_mux_address = 0x70;
    static constexpr int drive_metadata_mux_channel = 3;
    static constexpr int drive_presence_device_address = 0x61;
    static constexpr std::array<int, 10> drive_presence_map = {0, 1, 2, 3, 4,
                                                               5, 6, 7, 8, 9};

    static constexpr const char* drive_management_bus =
        "/sys/bus/i2c/devices/i2c-15";
    static constexpr std::array<int, 10> drive_mux_map = {
        0x70, 0x70, 0x70, 0x70, 0x71, 0x71, 0x71, 0x71, 0x72, 0x72};
    static constexpr std::array<int, 10> drive_channel_map = {0, 1, 2, 3, 0,
                                                              1, 2, 3, 0, 1};

    Inventory* inventory;
    const Bellavista* bellavista;
    std::array<gpiod::line, 10> lines;
    std::array<Connector<BasecampNVMeDrive>, 10> driveConnectors;
    std::array<PolledDevicePresence<BasecampNVMeDrive>, 10> presenceAdaptors;
};

class Bellavista : public Device, FRU
{
  public:
    explicit Bellavista(Inventory* inventory);
    Bellavista(const Bellavista& other) = delete;
    Bellavista(const Bellavista&& other) = delete;
    ~Bellavista() = default;

    Bellavista& operator=(const Bellavista& other) = delete;
    Bellavista& operator=(const Bellavista&& other) = delete;

    /* Device */
    virtual void plug(Notifier& notifier) override;
    virtual void unplug(Notifier& notifier,
                        int mode = Device::UNPLUG_REMOVES_INVENTORY) override;

    /* FRU */
    virtual std::string getInventoryPath() const override;
    virtual void addToInventory(Inventory* inventory) override;
    virtual void removeFromInventory(Inventory* inventory) override;

  private:
    static constexpr const char* basecamp_presence_device_path =
        "/sys/bus/i2c/devices/0-0062";
    static constexpr int basecamp_presence_offset = 12;

    Inventory* inventory;
    gpiod::chip basecampPresenceChip;
    gpiod::line basecampPresenceLine;
    Connector<Basecamp> basecampConnector;
    PolledDevicePresence<Basecamp> presenceAdaptor;
};

class Tola : public Device
{
  public:
    explicit Tola(Inventory* inventory);
    Tola(const Tola& other) = delete;
    Tola(const Tola&& other) = delete;
    ~Tola() = default;

    Tola& operator=(const Tola& other) = delete;
    Tola& operator=(const Tola&& other) = delete;

    /* Device */
    virtual void plug(Notifier& notifier) override;
    virtual void unplug(Notifier& notifier,
                        int mode = Device::UNPLUG_REMOVES_INVENTORY) override;

  private:
    Inventory* inventory;
    Bellavista bellavista;
};

class Everest : public Platform
{
  public:
    Everest() = default;
    ~Everest() = default;

    /* Platform */
    virtual void enrollWith(PlatformManager& pm) override;
    virtual void detectFrus(Notifier& notifier, Inventory* inventory) override;
};