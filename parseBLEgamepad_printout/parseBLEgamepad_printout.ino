#include "set.h" //引用实现了父类回调纯虚函数的头文件

ClientCallbacks clientCB;

int padData[11];
bool up_btn, dw_btn, lf_btn, rg_btn, x_btn, y_btn, a_btn, b_btn;
bool L1_btn, R1_btn, L2_btn, R2_btn, M1_btn, M2_btn;
bool START, SELECT, HOME, joyL_btn, joyR_btn;
int joystick_LX, joystick_LY, joystick_RX, joystick_RY;

void startScan()
{
  scanning = true;
  auto pScan = NimBLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
  pScan->setInterval(45);
  pScan->setWindow(35);
  Serial.println("Start scan");
  pScan->start(scanTime, scanEndedCB);
}

void scanEndedCB(NimBLEScanResults results) // 扫描结束回调函数
{
  scanning = false; // 扫描状态赋值 假
}

void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) // 收到通知回调函数
{
  static bool isPrinting = false;     // 声明为静态变量，并初始化，以便出函数体作用域后驻留内容中，当全局变量使用
  static unsigned long printedAt = 0; // 声明为静态变量，并初始化，以便出函数体作用域后驻留内容中，当全局变量使用
  if (isPrinting || millis() - printedAt < printInterval)
    return;                    // 以打印或时间未到则跳出函数体
  isPrinting = true;           // 打印状态赋 真
  for (int i = 3; i < 12; ++i) // 调试说明：将所有pData[]数组输出查看自己手柄输出哪几位是对应手柄各按键和摇杆的值，不同手柄不同的。
  {
    // Serial.printf("%d/", i);          // 数据位显示
    // Serial.printf("%03d ", pData[i]); // 遍历打印3-12位数据，每个数据占3个位置，便于观察测试输出的变化。
    padData[i] = pData[i]; // 传值到全局变量，记录手柄数据用于解析出各按键和摇杆值
  }
  parse_data();
  // Serial.println("");
  printedAt = millis(); // 更新打印时间变量到当前时间
  isPrinting = false;   // 打印状态赋 假
  print_value();
}

void charaSubscribeNotification(NimBLERemoteCharacteristic *pChara) // 订阅通知函数
{
  if (pChara->canNotify()) // 判断服务特征是否有Notify属性
  {
    Serial.println(" canNotify ");
    if (pChara->subscribe(true, notifyCB, true)) // 订阅通知
    {
      Serial.println("set notifyCb");
    }
  }
}

bool afterConnect(NimBLEClient *pClient)
{
  for (auto pService : *pClient->getServices(true)) // 遍历服务项目，寻找需要的服务UUID
  {
    auto sUuid = pService->getUUID(); // 逐一获取服务中的uuid
    if (!sUuid.equals(ServiceUUID))   // 判断是否有所需的目标uuid
    {
      continue; // 没有，则直接跳出函数体
    }
    for (auto pChara : *pService->getCharacteristics(true)) // 遍历服务的Characteristics
    {
      charaSubscribeNotification(pChara); // 判断服务特征是否有Notify属性，并订阅
    }
  }
  return true;
}

bool connectToServer(NimBLEAdvertisedDevice *advDevice)
{
  NimBLEClient *pClient = nullptr; // 创建空客户端实例，用空指针占位

  if (NimBLEDevice::getClientListSize())
  {
    pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
    if (pClient)
    {
      pClient->connect();
    }
  }

  if (!pClient)
  {
    pClient = NimBLEDevice::createClient();
    Serial.println("New client created");
    pClient->setClientCallbacks(&clientCB, false);
    pClient->setConnectTimeout(5);
    pClient->connect(advDevice, false);
  }

  Serial.print("Connected to: ");
  Serial.println(pClient->getPeerAddress().toString().c_str());

  bool result = afterConnect(pClient);
  if (!result)
  {
    return result;
  }
  return true;
}

void update_data() // 确保连接，更新数据
{
  if (!connected)
  {
    if (advDevice != nullptr)
    {
      if (connectToServer(advDevice))
      {
        Serial.println("成功! 现在可以接受解析手柄ble蓝牙信号了");
      }
      advDevice = nullptr;
    }
    else if (!scanning)
    {
      startScan();
    }
  }
}

void parse_data() // 解析padData[11]数据，更新各按键和摇杆数值
{
  a_btn = b_btn = x_btn = y_btn = false;
  up_btn = dw_btn = lf_btn = rg_btn = false;
  START = SELECT = HOME = false;

  joystick_LX = padData[3];
  joystick_LY = padData[4];
  joystick_RX = padData[5];
  joystick_RY = padData[6];

  switch (padData[7])
  {
  case 1:
    up_btn = true;
    break;
  case 2:
    dw_btn = true;
    break;
  case 4:
    lf_btn = true;
    break;
  case 8:
    rg_btn = true;
    break;
  case 16:
    a_btn = true;
    break;
  case 32:
    b_btn = true;
    break;
  case 64:
    x_btn = true;
    break;
  case 128:
    y_btn = true;
    break;
  }

  switch (padData[8])
  {
  case 1:
    L1_btn = true;
    break;
  case 2:
    L2_btn = true;
    break;
  case 16:
    SELECT = true;
    break;
  case 32:
    START = true;
    break;
  case 64:
    HOME = true;
    break;
  case 4:
    joyL_btn = true;
    break;
  case 8:
    joyR_btn = true;
    break;
  }
}

void print_value()
{
  if (up_btn)
  {
    Serial.println("上键press");
  }
  if (dw_btn)
  {
    Serial.println("下键press");
  }
  if (lf_btn)
  {
    Serial.println("左键press");
  }
  if (rg_btn)
  {
    Serial.println("右键press");
  }
  if (a_btn)
  {
    Serial.println("A press");
  }
  if (b_btn)
  {
    Serial.println("B press");
  }
  if (x_btn)
  {
    Serial.println("X press");
  }
  if (y_btn)
  {
    Serial.println("Y press");
  }
  if (SELECT)
  {
    Serial.println("SELECT press");
  }
  if (START)
  {
    Serial.println("START press");
  }
  if (HOME)
  {
    Serial.println("HOME press");
  }

  if (joystick_LX > 130 || joystick_LX < 126)
  {
    Serial.print("joystick_LX ");
    Serial.println(joystick_LX);
  }
  if (joystick_LY > 130 || joystick_LY < 126)
  {
    Serial.print("joystick_LY ");
    Serial.println(joystick_LY);
  }
  if (joystick_RX > 130 || joystick_RX < 126)
  {
    Serial.print("joystick_RX ");
    Serial.println(joystick_RX);
  }
  if (joystick_RY > 130 || joystick_RY < 126)
  {
    Serial.print("joystick_RY ");
    Serial.println(joystick_RY);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("设备启动...");
  NimBLEDevice::init("Esp32Car");
}

void loop()
{
  update_data();
}
