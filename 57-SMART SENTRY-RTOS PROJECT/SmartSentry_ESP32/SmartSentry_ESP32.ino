/**
 * @file    SmartSentry_ESP32.ino
 * @brief   ESP32 WiFi + Bluetooth bridge for Smart Sentry STM32 system.
 *
 * @details Receives alert messages from STM32 via UART (USART3) and
 *          forwards them to:
 *            - A TCP server on PC (PowerShell listener on port 8080)
 *            - A Bluetooth Classic serial client (e.g. Serial Bluetooth
 *              Terminal app on Android/iOS)
 *
 *          Protocol STM32 → ESP32:
 *            "WIFI:<message>\n"  → stripped to <message>, sent via TCP
 *            "BT:<message>\n"    → stripped to <message>, sent via BT
 *
 *          Protocol ESP32 → STM32:
 *            "CMD:SILENCE\n"     → silence all alerts
 *            "CMD:LIGHTS_ON\n"   → turn LED on
 *            "CMD:LIGHTS_OFF\n"  → turn LED off
 *            "CMD:STATUS\n"      → request system status
 *
 * @note    PowerShell TCP server command (run BEFORE powering STM32):
 * @code
 *   $l=[System.Net.Sockets.TcpListener]::new(8080); $l.Start()
 *   $c=$l.AcceptTcpClient()
 *   $r=New-Object System.IO.StreamReader($c.GetStream())
 *   while($c.Connected){$line=$r.ReadLine();if($line){Write-Host "ALERT: $line"}}
 * @endcode
 *
 * Wiring:
 *   ESP32 GPIO16 (RX2) ← STM32 PB10 (USART3_TX)
 *   ESP32 GPIO17 (TX2) → STM32 PB11 (USART3_RX)
 *   Common GND required.
 */

#include <WiFi.h>
#include "BluetoothSerial.h"
//#include "BluetoothSerial.h"

/* ── Configuration – update these for your network ──────────── */
const char* WIFI_SSID     = "keshav";   /**< WiFi network name    */
const char* WIFI_PASSWORD = "0987654321";         /**< WiFi password        */
const char* SERVER_IP     = "10.185.159.94";     /**< PC IP for TCP server */
const int   SERVER_PORT   = 8080;               /**< TCP port             */

/**
 * @brief  Set to 1 to enable TCP forwarding to PC.
 *         Set to 0 for Bluetooth-only mode (no PC server needed).
 */
#define TCP_ENABLED  1

/* ── UART pins for STM32 communication ──────────────────────── */
#define STM_RX  16  /**< ESP32 GPIO16 ← STM32 PB10 (USART3_TX) */
#define STM_TX  17  /**< ESP32 GPIO17 → STM32 PB11 (USART3_RX) */

HardwareSerial  SerialSTM(2);   /**< UART2 mapped to GPIO16/17 */
BluetoothSerial SerialBT;       /**< Classic BT serial port    */
WiFiClient      tcpClient;      /**< TCP client to PC server   */
bool            wifiOk = false; /**< WiFi connection status    */

/* ── Message filter ──────────────────────────────────────────── */
/**
 * @brief  Returns true only for messages this ESP32 should process.
 *         Drops all internal STM32 debug logs (no WIFI:/BT:/CMD: prefix).
 * @param  s  Message string received from STM32.
 * @return true if message should be forwarded.
 */
bool isValid(const String &s)
{
    return s.startsWith("WIFI:") ||
           s.startsWith("BT:")   ||
           s.startsWith("CMD:");
}

/* ── WiFi + TCP connection ───────────────────────────────────── */
/**
 * @brief  Connect to WiFi and optionally to the TCP server.
 *         Sends status strings back to STM32 via SerialSTM.
 */
void connectWiFi()
{
    Serial.print("[WiFi] Connecting to "); Serial.println(WIFI_SSID);
    WiFi.disconnect(true);
    delay(300);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int t = 0;
    while (WiFi.status() != WL_CONNECTED && t < 30)
    { delay(500); Serial.print("."); t++; }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        wifiOk = true;
        Serial.print("[WiFi] IP: "); Serial.println(WiFi.localIP());
        SerialSTM.print("ESP32:WIFI_OK\n");

#if TCP_ENABLED
        Serial.print("[TCP] Connecting to ");
        Serial.print(SERVER_IP); Serial.print(":"); Serial.println(SERVER_PORT);
        if (tcpClient.connect(SERVER_IP, SERVER_PORT))
        {
            Serial.println("[TCP] Connected!");
            SerialSTM.print("ESP32:TCP_OK\n");
        }
        else
        {
            Serial.println("[TCP] FAILED – start PowerShell server first!");
            SerialSTM.print("ESP32:TCP_FAIL\n");
        }
#endif
    }
    else
    {
        wifiOk = false;
        Serial.println("[WiFi] FAILED!");
        SerialSTM.print("ESP32:WIFI_FAIL\n");
    }
}

/* ── Arduino setup ───────────────────────────────────────────── */
/**
 * @brief  Arduino setup: initialise UART, Bluetooth, WiFi, then
 *         send ESP32:READY to STM32 to unblock the ESP32 task.
 */
void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("=========================================");
    Serial.println("  SmartSentry ESP32  – Bridge Firmware  ");
    Serial.println("=========================================");

    /* UART to STM32 */
    SerialSTM.begin(115200, SERIAL_8N1, STM_RX, STM_TX);
    Serial.println("[UART] GPIO16(RX)<-PB10  GPIO17(TX)->PB11");

    /* Bluetooth Classic */
    if (SerialBT.begin("SmartSentry"))
        Serial.println("[BT] Ready. Device name: SmartSentry");
    else
        Serial.println("[BT] FAILED!");
    Serial.println("[BT] Commands: STATUS  LIGHTS_ON  LIGHTS_OFF  SILENCE");

    /* WiFi + TCP */
    connectWiFi();

    /* Signal STM32 that bridge is ready */
    delay(200);
    SerialSTM.print("ESP32:READY\n");
    Serial.println("[ESP32] READY sent to STM32.");
}

/* ── Arduino main loop ───────────────────────────────────────── */
/**
 * @brief  Main loop: route messages between STM32, TCP, and Bluetooth.
 *
 * @details Three data paths:
 *  1. STM32 → ESP32: filter valid messages, forward WIFI: to TCP,
 *     forward BT: to Bluetooth.
 *  2. Bluetooth phone → STM32: prefix CMD: and forward.
 *  3. TCP server → STM32: prefix CMD: and forward.
 *  WiFi keepalive check every 30 s.
 */
void loop()
{
    /* ── Path 1: STM32 → TCP / BT ───────────────────────────── */
    if (SerialSTM.available())
    {
        String msg = SerialSTM.readStringUntil('\n');
        msg.trim();
        if (msg.length() == 0) return;
        if (!isValid(msg)) return;  /* drop STM32 internal debug logs */

        Serial.print("[STM32>>] "); Serial.println(msg);

        /* WIFI:<payload> → TCP server */
        if (msg.startsWith("WIFI:"))
        {
            String payload = msg.substring(5);
#if TCP_ENABLED
            if (!tcpClient.connected())
            {
                Serial.println("[TCP] Reconnecting...");
                tcpClient.connect(SERVER_IP, SERVER_PORT);
            }
            if (tcpClient.connected())
            {
                tcpClient.print(payload);
                tcpClient.print("\n");
                Serial.print("[TCP>>] "); Serial.println(payload);
            }
            else
                Serial.println("[TCP] Not connected.");
#endif
        }

        /* BT:<payload> → Bluetooth phone */
        if (msg.startsWith("BT:"))
        {
            String payload = msg.substring(3);
            SerialBT.print(payload);
            SerialBT.print("\n");
            Serial.print("[BT>>] "); Serial.println(payload);
        }
    }

    /* ── Path 2: Bluetooth phone → STM32 ────────────────────── */
    if (SerialBT.available())
    {
        String cmd = SerialBT.readStringUntil('\n');
        cmd.trim();
        if (cmd.length() > 0)
        {
            Serial.print("[BT IN] "); Serial.println(cmd);
            SerialSTM.print("CMD:"); SerialSTM.print(cmd); SerialSTM.print("\n");
            SerialBT.print("ACK: "); SerialBT.println(cmd);
        }
    }

    /* ── Path 3: TCP server → STM32 ─────────────────────────── */
#if TCP_ENABLED
    if (tcpClient.available())
    {
        String cmd = tcpClient.readStringUntil('\n');
        cmd.trim();
        if (cmd.length() > 0)
        {
            Serial.print("[TCP IN] "); Serial.println(cmd);
            SerialSTM.print("CMD:"); SerialSTM.print(cmd); SerialSTM.print("\n");
        }
    }
#endif

    /* ── WiFi keepalive every 30 s ───────────────────────────── */
    static unsigned long lastChk = 0;
    if (millis() - lastChk > 30000)
    {
        lastChk = millis();
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("[WiFi] Lost – reconnecting...");
            connectWiFi();
        }
        else
        {
            Serial.print("[WiFi] OK RSSI=");
            Serial.print(WiFi.RSSI());
            Serial.println("dBm");
        }
    }

    delay(10);
}
