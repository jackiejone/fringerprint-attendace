/*This code switches the RFID-RC522 between read and write mode when a switch conntected to pin D4 is pressed.
  The code for reading and writing to the RFID tags is derived from examples from a RFID custom library
  by miguelbalboa from his RFID github Repository (https://github.com/miguelbalboa/rfid.git)
  The code also sends the data scanned from an RFID tag to a server over WIFI
  by Jackie Jone

  When uploading code to Wemos, you need to disconnect pins D0, D4
*/

// Defining Pin Numbers
#define RST_PIN D3   // Reset Pin for MFRC522
#define SS_PIN D8    // Slave Select Pin
#define RED_LED D0   // LED pin for indicating read or write mode
#define switchPin D4 // Pin for switch to change read and write mode

// Network Information
#define STASSID "RaspberryPiNetwork" // Network Name
#define STAPSK  "password"           // Network Password

// Setting LCD address for I2C
LiquidCrystal_I2C lcd(0x27,20,4);

// Create MFRC522 instance for RFID scanner
MFRC522 mfrc522(SS_PIN, RST_PIN);   

// Seting networking variables
const char* ssid     = STASSID;    // Network Name
const char* password = STAPSK;     // Network Password
const char* host = "192.168.4.1";  // Server IP
const uint16_t port = 80;          // Server Port

// Network connecting message
String message = "Connecting to: " + String(ssid); 

// Initialisation of system
void setup() {
  // Setting up Network connection / Connecting to Network as a WIFI client
  Serial.begin(115200);                                     // Initialize serial monitor
  lcd.init();                                               // Initialize the LCD
  lcd.backlight();                                          // Turn on LCD backlight
  lcd.clear();                                              // Clear anything on LCD display
  lcd.print(message);                                       // Show network connection message on LCD
  Serial.print("\nConnecting to: ");                        // Printing to Serial monitor the Network the system is connecting to
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);                                      // Setting ESP8266 as a WIFI client
  WiFi.begin(ssid, password);                               // Beginning Network connection

  // Loop to print "." to serial monitor while connecting to the network
  while (WiFi.status() != WL_CONNECTED) {                   // Checking if the ESP8266 is not connected to the network
    lcd.scrollDisplayLeft();                                // Scrolling the message on LCD
    Serial.print(".");                                      // Printing "." to serial monitor
    delay(300);                                             // 0.3 seconds of delay
  }
  
  Serial.println("");                                       // Printing new line to serial monitor
  Serial.println("WiFi connected");                         // Prints connection message to serial monitor
  Serial.println("IP address: ");                           // Prints local IP address assigned to WIFI client
  Serial.println(WiFi.localIP());
  lcd.clear();                                              // Clears any messages on LCD
  lcd.setCursor(0,0);                                       // Sets LCD cursor to beginning

  pinMode(switchPin, INPUT);                                // Sets pinmode of switch pin
  SPI.begin();                                              // Initialize SPI bus for communication to MFRC522
  mfrc522.PCD_Init();                                       // Initialize the MFRC522
  pinMode(RED_LED, OUTPUT);                                 // Sets LED pin to output
}

// Looping main functions
void loop() {
  // Checks the state of the pin connected to the switch to switch between reading and writing mode
  if (digitalRead(switchPin) == LOW) {                      // If the pin is LOW, put the system into Read Mode
    digitalWrite(RED_LED, LOW);                             // Turns red LED off to show that the system is in read mode
    RFID_read();                                            // Runs function for reading RFID tag
    lcd.clear();                                            // Clears anything on LCD
    lcd.print("Mode: Read");                                // Prints on the LCD that the system is in read mode
    delay(500);                                             // 0.5 seconds of dealy
  } else if (digitalRead(switchPin) == HIGH) {              // If the pin is HIGH, put the system into Write Mode
    digitalWrite(RED_LED, HIGH);                            // Turns the red LED on to show that the system is in write mode
    RFID_write();                                           // Runs function for writing to RFID tag 
    lcd.clear();                                            // Clears and message on the LCD
    lcd.print("Mode: Write");                               // Prints on the lcd that the system is in write mode
    delay(500);                                             // 0.5 seconds of delay
  }
}

// Custom function for sending data to server
void send_data(const String uid, const String user) {
    HTTPClient http;                                                           // Begins HTTP client
    String httpRequestData = "user=" + user + "&uid=" + uid;                   // Creates a string with all the data and to send to the server
    http.begin("http://192.168.4.1/insert");                                   // Defines the link to the web server which the data is to be sent to
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");       // Defines header for JSON request sent to the server
    Serial.print("\nhttpRequestData: ");                                       // Prints out what is going to be sent to the server
    Serial.print(httpRequestData);

    int httpResponseCode = http.POST(httpRequestData);                         // Sends the data to the server and gets the HTTP response code
    if (httpResponseCode>0) {                                                  // Checks if the response code is greater than 0 (Meaning that the messasge was successfully sent)
      Serial.println("HTTP Response code: ");                                  // Prints the reponse code to the serial monitor
      Serial.print(httpResponseCode);
    }
    else {                                                                     // If the HTTP repose code is not greater than 0 then an error has occured
      Serial.println("Error code: ");                                          // Prints HTTP response code to the serial monitor
      Serial.print(httpResponseCode);
      }
  }

// Custom function to get usernames and ids to write to an RFID chip
  String get_data() {
  
  }


// Function for reading data (Name and ID) off RFID chip
// Most of the code for this is taken from the MFRC522 example (rfid_read_personal_data)
void RFID_read() {
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards (MFRC522 is capable  of reading multiple cards ontop of each other)
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("**Card Detected:**"));

  //-------------------------------------------

  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card to serial monitor


  //------------------------------------------- Get student ID from the card
  Serial.print(F("Student ID: "));

  byte buffer1[18];

  block = 4;
  len = 18;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Getting student ID from the card
  char usrid[] = "";
  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer1[i] != 32)
    {
      usrid[i] = buffer1[i];            // Appending characters of the student ID to a variable
    }
  }
  
  Serial.print("\nUSRID string: ");     // Printing student ID to the serial monitor
  Serial.print(usrid);
  lcd.clear();                          // Clearing anything off the LCD
  lcd.setCursor(0,0);                   // Setting LCD cursor to 0, 0 (row 0, column 0)
  lcd.print("User ID: ");               // Printing user ID to the LCD
  lcd.print(usrid);
  Serial.print("\n");
  String userid;
  userid = String(usrid);               // Turning char array of user ID into a string

  //---------------------------------------- Getting first name from RFID chip

  Serial.print(F("Name: \n"));

  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Getting student id from the card
  char uname[] = "";
  for (uint8_t i = 0; i < 16; i++) {
    uname[i] = buffer2[i];              // Getting username from the card
  }
  
  Serial.print("\nFist name: ");        // Printing name to serial monitor
  Serial.println(uname);
  lcd.setCursor(0,1);                   // Setting LCD cursor to 0, 1 (row 0, column 1)
  lcd.print("Username: ");              // Printing username to the LCD
  lcd.print(uname);
  lcd.setCursor(0,0);                   // Setting LCD cursor back to start
  String username;
  username = String(uname);             // Turning char array into string
  send_data(userid, username);          // Running function to send the data from the rfid chip to server
  delay(1000);                          // 1 second of delay
  //----------------------------------------

  Serial.println(F("\n**End Reading**\n"));        

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}


// Custom function for writing to the RFID chip
void RFID_write() {
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print(F("Card UID:"));                                                     // Prints UID of card to serial monitor
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print(F(" PICC type: "));                                                  // Prints card PICC type to serial monitor
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  byte buffer[34];
  byte block;
  MFRC522::StatusCode status;
  byte len;

  Serial.setTimeout(10000L) ;                                                       // wait until 10 seconds for input from serial
                                                                                    // Ask personal data: first
  Serial.println(F("Type name, ending with #"));
  len = Serial.readBytesUntil('#', (char *) buffer, 30) ;                           // read user name from serial
  for (byte i = len; i < 30; i++) buffer[i] = ' ';                                  // pad with spaces

  block = 1;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("PCD_Authenticate() success: "));

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  block = 2;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

                                                                // Ask personal data: student number or user id
  Serial.println(F("Type student number, ending with #"));
  len = Serial.readBytesUntil('#', (char *) buffer, 20) ;       // read user id from serial
  for (byte i = len; i < 20; i++) buffer[i] = ' ';              // pad with spaces

  block = 4;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  block = 5;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));


  Serial.println(" ");
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
}
