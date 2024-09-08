#include <bluefruit.h>

BLEClientBas  clientBas;  // battery client
BLEClientDis  clientDis;  // device information client
BLEClientUart clientUart; // bleuart client

void setup()
{
  Serial.begin(115200);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println("Bluefruit52 Central BLEUART Example");
  Serial.println("-----------------------------------\n");
  
  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  Bluefruit.begin(0, 1);
  
  Bluefruit.setName("Bluefruit52 Central");

  // Configure Battery client
  clientBas.begin();  

  // Configure DIS client
  clientDis.begin();

  // Init BLE Central Uart Service
  clientUart.begin();
  clientUart.setRxCallback(bleuart_rx_callback);

  // Increase Blink rate to different from PrPh advertising mode
  Bluefruit.setConnLedInterval(250);

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(connect_callback);
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);

  // Start Central Scanning
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
  Bluefruit.Scanner.useActiveScan(false);
  Bluefruit.Scanner.start(0);  // 0 = Don't stop scanning after n seconds
}

void scan_callback(ble_gap_evt_adv_report_t* report)
{
  // Check if advertising contains BleUart service
  if ( Bluefruit.Scanner.checkReportForService(report, clientUart) )
  {
    Serial.print("BLE UART service detected. Connecting ... ");
    // Connect to device with bleuart service in advertising
    Bluefruit.Central.connect(report);
  }
  else
  {
    // For Softdevice v6: after received a report, scanner will be paused
    // We need to call Scanner.resume() to continue scanning
    Bluefruit.Scanner.resume();
  }
}

void connect_callback(uint16_t conn_handle)
{
  Serial.println("Connected");

  Serial.print("Discovering Device Information ... ");
  if (clientDis.discover(conn_handle))
  {
    Serial.println("Found it");
    char buffer[32+1];
    
    // Read and print out Manufacturer
    memset(buffer, 0, sizeof(buffer));
    if (clientDis.getManufacturer(buffer, sizeof(buffer)))
    {
      Serial.print("Manufacturer: ");
      Serial.println(buffer);
    }

    // Read and print out Model Number
    memset(buffer, 0, sizeof(buffer));
    if (clientDis.getModel(buffer, sizeof(buffer)))
    {
      Serial.print("Model: ");
      Serial.println(buffer);
    }

    Serial.println();
  }
  else
  {
    Serial.println("Found NONE");
  }

  Serial.print("Discovering BLE Uart Service ... ");
  if (clientUart.discover(conn_handle))
  {
    Serial.println("Found it");
    Serial.println("Enable TXD's notify");
    clientUart.enableTXD();
    Serial.println("Ready to receive from peripheral");
  }
  else
  {
    Serial.println("Found NONE");
    Bluefruit.disconnect(conn_handle);  // Disconnect if the BLE UART service is not found
  }
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}

void bleuart_rx_callback(BLEClientUart& uart_svc)
{
  static String received = "";  // Static to keep the data between calls

  while (uart_svc.available())
  {
    char c = (char)uart_svc.read();
    if (c == '\n')  // Check for end of message
    {
      Serial.println(received);  // Process complete message
      received = "";  // Clear buffer
    }
    else
    {
      received += c;  // Add character to buffer
    }
  }
}

void loop()
{
  if (Bluefruit.Central.connected())
  {
    if (clientUart.discovered())
    {
      if (Serial.available())
      {
        delay(2); 
        char str[21] = {0};
        Serial.readBytes(str, 20);
        clientUart.print(str);
      }
    }
  }
}
