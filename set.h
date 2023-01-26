#pragma once // 加入该指令的文件只会被编译一次
#include <NimBLEDevice.h>

NimBLEAddress Address("03:66:00:29:27:68");                     // 蓝牙手柄的地址
NimBLEUUID ServiceUUID("91680001-1111-6666-8888-0123456789ab"); // 蓝牙手柄有数据输出的服务UUID

NimBLEAdvertisedDevice *advDevice; // 生成广播设备实例的指针

bool scanning = false, connected = false; // 记录扫描和连接状态
int scanTime = 0, printInterval = 100;    // scanTime单位是秒，0表示一直扫描. printInterval为打印输出间隔时间

class ClientCallbacks : public NimBLEClientCallbacks // 父类中的onConnect和onDisconnetc为纯虚函数，此处需重载添加具体实现。
{
    void onConnect(NimBLEClient *pClient)
    {
        Serial.println("连接成功");
        connected = true;
    }
    void onDisconnect(NimBLEClient *pClient)
    {
        connected = false;
    }
};

class AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks // 重载父类中的onResult纯虚函数
{
    void onResult(NimBLEAdvertisedDevice *advertisedDevice)
    {
        if (advertisedDevice->getAddress() == Address)
        // if (advertisedDevice->isAdvertisingService(ServiceUUID))
        {
            Serial.println("发现蓝牙手柄");
            NimBLEDevice::getScan()->stop();
            advDevice = advertisedDevice;
        }
    }
};