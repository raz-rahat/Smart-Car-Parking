# 🅿️ Smart Car Parking System (ESP32 + RFID + Web Dashboard)

<p align="center">
  <img src="https://img.shields.io/badge/Board-ESP32%20Dev%20Module-red?style=for-the-badge&logo=expressif" alt="ESP32">
  <img src="https://img.shields.io/badge/Access-RFID%20MFRC522-blue?style=for-the-badge" alt="RFID">
  <img src="https://img.shields.io/badge/Interface-WiFi%20Web%20Dashboard-green?style=for-the-badge" alt="WiFi Dashboard">
  <img src="https://img.shields.io/badge/Display-16x2%20I2C%20LCD-purple?style=for-the-badge" alt="LCD">
  <img src="https://img.shields.io/badge/Language-C%2B%2B%20%2F%20Arduino-00599C?style=for-the-badge&logo=cplusplus" alt="C++">
</p>

---

## 📌 প্রজেক্ট পরিচিতি (Project Overview)

**Smart Car Parking System** একটি সম্পূর্ণ অটোমেটেড ও IoT-সংযুক্ত গাড়ি পার্কিং ম্যানেজমেন্ট সিস্টেম, যা **ESP32**, **RFID কার্ড এক্সেস**, **আল্ট্রাসনিক ও IR সেন্সর**, **সার্ভো গেট**, **16x2 LCD ডিসপ্লে** এবং একটি **লাইভ ওয়েব ড্যাশবোর্ড** দিয়ে তৈরি।

সিস্টেমটি এন্ট্রি ও এক্সিট — দুই পাশেই আলাদা আল্ট্রাসনিক সেন্সর দিয়ে গাড়ি সনাক্ত করে, তারপর RFID কার্ড ভেরিফাই করে **রেজিস্টার্ড ইউজারদের** জন্য অটোমেটিক গেট খুলে দেয়। ৩টি IR সেন্সরের মাধ্যমে প্রতিটি পার্কিং স্লটের অবস্থা (ফাঁকা/দখল) রিয়েল-টাইমে ট্র্যাক করা হয়, এবং পার্কিং ফুল থাকলে নতুন এন্ট্রি প্রত্যাখ্যান করা হয়। পুরো সিস্টেমের স্ট্যাটাস LCD-তে এবং ESP32-এর নিজস্ব WiFi হটস্পট থেকে একটি সুন্দর **অ্যানিমেটেড ওয়েব ড্যাশবোর্ডে** দেখা যায়।

---

## ✨ প্রধান ফিচারসমূহ (Key Features)

- 🪪 **RFID কার্ড অথেন্টিকেশন:** MFRC522 মডিউল দিয়ে প্রি-রেজিস্টার্ড কার্ড UID ভেরিফাই করে এক্সেস দেওয়া হয়; অচেনা কার্ডে "Not Registered" ওয়ার্নিং।
- 🚧 **স্বয়ংক্রিয় সার্ভো বুম গেট:** ভেরিফাইড কার্ড পেলে গেট নিজে থেকে খুলে যায় এবং গাড়ি পার হয়ে গেলে বা নির্দিষ্ট সময় পর নিজে থেকে বন্ধ হয়ে যায়।
- 📡 **ডুয়াল আল্ট্রাসনিক সেন্সর (Entry / Exit):** একটি সেন্সর গাড়ি ঢোকা সনাক্ত করে, অন্যটি বের হওয়া — একই গেট দিয়ে Entry ও Exit উভয় ফ্লো পরিচালিত হয়।
- 🅿️ **৩-স্লট IR পার্কিং ডিটেকশন:** প্রতিটি স্লটে আলাদা IR সেন্সর দিয়ে রিয়েল-টাইমে ফাঁকা/দখল স্ট্যাটাস ট্র্যাক করা হয় এবং অ্যাভেইলেবল স্লট সংখ্যা অটো-ক্যালকুলেট হয়।
- 🚫 **পার্কিং ফুল প্রটেকশন:** সব স্লট দখল থাকলে ভেরিফাইড কার্ডধারী হলেও এন্ট্রি আটকে "No Space" মেসেজ দেখানো হয়।
- 🖥️ **16x2 I2C LCD স্ট্যাটাস ডিসপ্লে:** স্টার্টআপ স্প্ল্যাশ স্ক্রিন, IP অ্যাড্রেস, Welcome/Goodbye মেসেজ, এবং Entry/Exit/Fail কাউন্টার দেখায়।
- 🌐 **লাইভ ওয়েব ড্যাশবোর্ড (WiFi Access Point):** ইন্টারনেট ছাড়াই ESP32 নিজেই হটস্পট হয়ে একটি অ্যানিমেটেড ড্যাশবোর্ড সার্ভ করে — গেট স্ট্যাটাস, গাড়ি অ্যানিমেশন, LED ইন্ডিকেটর, স্লট গ্রিড ও স্ট্যাটিস্টিক্স সহ।
- 🎛️ **ম্যানুয়াল গেট কন্ট্রোল:** ড্যাশবোর্ড থেকেই **Open Gate**, **Close Gate** এবং **Reset Count** বাটন দিয়ে সিস্টেম নিয়ন্ত্রণ করা যায়।
- 🔊 **বাজার ও LED অ্যালার্ট প্যাটার্ন:** Success (1 বিপ), Entry/Exit সম্পন্ন (2 বিপ), Invalid Card (3 বিপ) — প্রতিটি ইভেন্টের জন্য আলাদা বিপ প্যাটার্ন ও Green/Yellow/Red LED ইঙ্গিত।
- ⏱️ **স্মার্ট টাইমআউট সিস্টেম:** কার্ড ভেরিফিকেশন (15s), গেট ওপেন টাইম (10s), গাড়ি পাস করার টাইমআউট (15s) — প্রতিটির জন্য আলাদা সেফটি টাইমার।

---

## 🧰 প্রয়োজনীয় উপাদান (Hardware Components)

1. **ESP32 Development Board**
2. **RFID Module — MFRC522** (+ কমপক্ষে ২টি RFID কার্ড/ট্যাগ)
3. **SG90 (বা সমমানের) Servo Motor** — বুম গেট
4. **HC-SR04 Ultrasonic Sensor × 2** — Entry ও Exit ডিটেকশন
5. **IR Obstacle/Proximity Sensor × 3** — পার্কিং স্লট ডিটেকশন
6. **16x2 I2C LCD Display (PCF8574, Address 0x27)**
7. **Active Buzzer Module**
8. **LED (Green, Red, Yellow)** + 220Ω রেজিস্টর
9. **USB ডাটা কেবল**
10. **Connecting/Jumper Wires ও ব্রেডবোর্ড**

---

## 📚 প্রয়োজনীয় সফটওয়্যার ও লাইব্রেরি (Libraries Required)

Arduino IDE-তে নিচের লাইব্রেরিগুলো ইনস্টল করা থাকতে হবে:

| লাইব্রেরির নাম (Library Name) | কাজ | Library Manager Search Term |
| :--- | :--- | :--- |
| **SPI.h** | RFID মডিউলের সাথে SPI কমিউনিকেশন | ESP32 Core-এর সাথে ডিফল্ট থাকে |
| **MFRC522** (by GithubCommunity) | RFID কার্ড রিড/রাইট | `MFRC522` |
| **ESP32Servo** | বুম গেট সার্ভো নিয়ন্ত্রণ | `ESP32Servo` |
| **Wire.h** | I2C কমিউনিকেশন (LCD-এর জন্য) | ESP32 Core-এর সাথে ডিফল্ট থাকে |
| **LiquidCrystal_I2C** (by Frank de Brabander / Marco Schwartz) | 16x2 I2C LCD নিয়ন্ত্রণ | `LiquidCrystal_I2C` |
| **WiFi.h** | ESP32-কে WiFi Access Point হিসেবে চালানো | ESP32 Core-এর সাথে ডিফল্ট থাকে |
| **WebServer.h** | HTTP ওয়েব সার্ভার ও ড্যাশবোর্ড হোস্টিং | ESP32 Core-এর সাথে ডিফল্ট থাকে |

---

## 🔌 পিন টু পিন কানেকশন (Detailed Pinout Diagram)

### 🪪 ১. RFID Module (MFRC522) → ESP32 Connection
| RFID Pin | ESP32 GPIO Pin | কাজ |
| :--- | :--- | :--- |
| **SDA (SS)** | **GPIO 5** | SPI Slave Select |
| **RST** | **GPIO 4** | Reset Pin |
| **SCK** | **GPIO 18** | SPI Clock |
| **MISO** | **GPIO 19** | SPI Data Out |
| **MOSI** | **GPIO 23** | SPI Data In |
| **VCC** | **3.3V** | পাওয়ার সাপ্লাই |
| **GND** | **GND** | গ্রাউন্ড |

### 🚧 ২. Servo Motor (Boom Gate) → ESP32 Connection
| Servo Wire | ESP32 GPIO Pin | কাজ |
| :--- | :--- | :--- |
| **Signal (হলুদ)** | **GPIO 13** | PWM কন্ট্রোল সিগন্যাল |
| **VCC (লাল)** | **5V / VIN** | পাওয়ার সাপ্লাই |
| **GND (বাদামি/কালো)** | **GND** | গ্রাউন্ড |

### 🖥️ ৩. I2C LCD Display (16x2) → ESP32 Connection
| LCD Pin | ESP32 GPIO Pin | কাজ |
| :--- | :--- | :--- |
| **SDA** | **GPIO 21** | I2C Data Line |
| **SCL** | **GPIO 22** | I2C Clock Line |
| **VCC** | **5V** | পাওয়ার সাপ্লাই |
| **GND** | **GND** | গ্রাউন্ড |

### 📏 ৪. Ultrasonic Sensor 1 (Entry / প্রবেশ) → ESP32 Connection
| HC-SR04 Pin | ESP32 GPIO Pin | কাজ |
| :--- | :--- | :--- |
| **TRIG** | **GPIO 27** | Trigger সিগন্যাল আউটপুট |
| **ECHO** | **GPIO 26** | Echo সিগন্যাল ইনপুট |
| **VCC** | **5V** | পাওয়ার সাপ্লাই |
| **GND** | **GND** | গ্রাউন্ড |

### 📏 ৫. Ultrasonic Sensor 2 (Exit / বের হওয়া) → ESP32 Connection
| HC-SR04 Pin | ESP32 GPIO Pin | কাজ |
| :--- | :--- | :--- |
| **TRIG** | **GPIO 25** | Trigger সিগন্যাল আউটপুট |
| **ECHO** | **GPIO 33** | Echo সিগন্যাল ইনপুট |
| **VCC** | **5V** | পাওয়ার সাপ্লাই |
| **GND** | **GND** | গ্রাউন্ড |

### 🅿️ ৬. IR Sensors (Parking Slots) → ESP32 Connection
| Slot | ESP32 GPIO Pin | কাজ |
| :--- | :--- | :--- |
| **Slot 1 Out** | **GPIO 32** | স্লট ১ অকুপেন্সি ডিটেকশন |
| **Slot 2 Out** | **GPIO 35** | স্লট ২ অকুপেন্সি ডিটেকশন |
| **Slot 3 Out** | **GPIO 34** | স্লট ৩ অকুপেন্সি ডিটেকশন |
| **VCC** | **5V / 3.3V** | পাওয়ার সাপ্লাই |
| **GND** | **GND** | গ্রাউন্ড |

### 🔔 ৭. LED ও Buzzer → ESP32 Connection
| কম্পোনেন্ট | ESP32 GPIO Pin | কাজ |
| :--- | :--- | :--- |
| **Green LED (+)** | **GPIO 12** | Access Granted / Verified ইঙ্গিত |
| **Red LED (+)** | **GPIO 15** | Access Denied / Not Registered ইঙ্গিত |
| **Yellow LED (+)** | **GPIO 2** | Parking Full ইঙ্গিত |
| **Buzzer (+)** | **GPIO 14** | বিপ অ্যালার্ট |
| **(সবগুলোর negative)** | **GND** | (রেজিস্টরসহ) গ্রাউন্ড |

---

## 📶 WiFi সংযোগ তথ্য (WiFi Connection Info)

কোড আপলোডের পর ESP32 নিজেই একটি WiFi হটস্পট তৈরি করবে:

| তথ্য | মান |
| :--- | :--- |
| **WiFi Network (SSID)** | `SmartParking` |
| **Password** | `12345678` |
| **Web Dashboard URL** | `http://192.168.4.1/` |

> ⚠️ **নিরাপত্তা নোট:** রিয়েল ব্যবহারের আগে `.ino` ফাইলে `AP_SSID` ও `AP_PASSWORD` পরিবর্তন করে নেওয়া ভালো।

---

## 🪪 RFID কার্ড রেজিস্ট্রেশন (Registered Cards)

কোডের মধ্যে বর্তমানে ২টি ডেমো কার্ড হার্ডকোড করা আছে:

| ইউজার | Card UID |
| :--- | :--- |
| **RAZ** | `F0B5F052` |
| **RAHAT** | `A199F70A` |

নতুন কার্ড যোগ করতে চাইলে সিরিয়াল মনিটরে (115200 baud) কার্ড স্ক্যান করে তার UID দেখে নিন, তারপর `.ino` ফাইলে `CARD*_UID` ও `USER*_NAME` ভ্যারিয়েবলে যোগ করুন।

---

## ⚙️ সিস্টেম কীভাবে কাজ করে (How It Works)

1. **Entry:** গাড়ি Sensor 1 (Entry) এর কাছে আসলে সিস্টেম RFID কার্ড চায়। ১৫ সেকেন্ডের মধ্যে ভেরিফাইড কার্ড স্ক্যান করলে ও পার্কিং-এ জায়গা থাকলে গেট খুলে যায়।
2. গাড়ি গেট পার হয়ে **Sensor 2** ক্রস করলে Entry সফল ধরা হয় এবং গেট বন্ধ হয়ে যায়।
3. **Exit:** গাড়ি Sensor 2 (Exit) এর কাছে আসলে একইভাবে কার্ড ভেরিফাই করে গেট খোলে, এবং Sensor 1 ক্রস করলে Exit সফল ধরা হয়।
4. পুরো সময় ৩টি IR সেন্সর দিয়ে স্লট অকুপেন্সি রিয়েল-টাইমে আপডেট হতে থাকে এবং Available Slot কাউন্ট দেখানো হয়।
5. অচেনা কার্ড, টাইমআউট বা পার্কিং ফুল হলে LCD, LED ও বাজারে আলাদা আলাদা এরর ইঙ্গিত দেখানো হয়।
6. সব ইভেন্ট (Entry/Exit/Fail কাউন্ট, স্লট স্ট্যাটাস, গেট স্ট্যাটাস, শেষ কার্ড UID) প্রতি ০.৫ সেকেন্ডে ওয়েব ড্যাশবোর্ডে লাইভ আপডেট হয়।

---

## 🖥️ ওয়েব ড্যাশবোর্ড ফিচার (Web Dashboard)

- 🚗 **অ্যানিমেটেড গেট ও গাড়ি সিন:** গেট খোলা/বন্ধ ও গাড়ি ঢোকা/বের হওয়ার লাইভ ভিজ্যুয়াল অ্যানিমেশন।
- 🟢🟡🔴 **LED স্ট্যাটাস ইন্ডিকেটর:** Access Granted, Parking Full, Access Denied এর জন্য আলাদা রঙের গ্লো ইফেক্ট।
- 📊 **রিয়েল-টাইম স্ট্যাটিস্টিক্স:** Free Slot, Total Entries, Total Exits, Verify Fail কাউন্ট।
- 🅿️ **স্লট গ্রিড:** প্রতিটি স্লটের Free/Occupied স্ট্যাটাস আইকনসহ, পরিবর্তন হলে ফ্ল্যাশ অ্যানিমেশন।
- 🎛️ **কন্ট্রোল বাটন:** Open Gate / Close Gate / Reset Count।
- 📝 **Last Event প্যানেল:** সর্বশেষ কার্ড UID, অ্যাক্সেস রেজাল্ট ও সিস্টেম মেসেজ।
- 🔌 **Connection Status:** ESP32-এর সাথে সংযোগ বিচ্ছিন্ন হলে "CONNECTION LOST" নোটিফিকেশন।

---

## ⚙️ আর্ডুইনো আইডিই আপলোড সেটআপ (Arduino IDE Setup)

1. **Board:** `ESP32 Dev Module`
2. **Upload Speed:** `115200`
3. **Partition Scheme:** `Default`
4. উপরে উল্লেখিত সবগুলো লাইব্রেরি ইনস্টল করুন।
5. সঠিক **COM Port** সিলেক্ট করে `Upload` চাপুন।
6. আপলোড শেষে Serial Monitor (115200 baud) ওপেন করলে WiFi SSID ও ড্যাশবোর্ডের IP Address দেখতে পাবেন।

---

## 📂 ফাইল স্ট্রাকচার (Project Structure)

```
📦 Smart_Car_Parking_System
 ┗ 📜 smart_parking_system_2_modified.ino   → পুরো লজিক, স্টেট মেশিন, RFID/সেন্সর হ্যান্ডলিং ও এমবেডেড ওয়েব ড্যাশবোর্ড
```

---

## 📺 ভিডিও টিউটোরিয়াল ও চ্যানেল (YouTube & Support)

<p align="center">
  <a href="https://youtu.be/0Nb7TMpkb_0?si=voNOm6EwzIhUyrkx" target="_blank">
    <img src="https://img.shields.io/badge/YouTube-Watch%20Demo-red?style=for-the-badge&logo=youtube" alt="YouTube Demo">
  </a>
</p>

প্রোজেক্টটি আপনার কাজে আসলে গিটহাবে একটি **Star (⭐)** দিয়ে পাশে থাকবেন। Happy Hacking! 🅿️✨
