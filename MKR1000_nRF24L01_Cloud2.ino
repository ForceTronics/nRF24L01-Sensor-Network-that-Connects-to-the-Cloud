/*10/24/16 This code was written for the YouTube video "Creating a Sensor Network that Connects to the Cloud Part 2"
 * on the ForceTronics channel. This code is public domain and free to others to use and modify at your own risk
 */
#include <RF24.h> //nRF24L01 library: https://github.com/TMRh20/RF24
#include <RF24NetworkNoSleep.h> //modified verision of the RF24Network library to work on non-AVR platform
#include <SPI.h> 
#include <WiFi101.h> //used for WiFi on MKR1000 and WiFi shield
#include <RTCZero.h> //used to access RTC capabilities of MKR1000
#include "PhantMKR1K.h" //library was leveraged from Sparkfun's Phant library and modified to work on Arduino MKR1000

RTCZero rtc; //create real time clock object
char ssid[] = "NETGEAR42"; //  your network SSID (name)
char pass[] = "excitedstreet973";    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS; //set status variable
const int LED_PIN = 6; // MKR1000's built-in LED

//define areas for phant cloud address and security keys
const char PhantHost[] = "data.sparkfun.com";
const char PublicKey[] = "NJO4a03mWjtv0M3JyVL3";
const char PrivateKey[] = "5dZx6KNnPGCXwDY2jAxY";

// start RF24 communication layer
RF24 radio(5,7);

// start RF24 network layer
RF24NetworkNoSleep network(radio);

// Coordinator address
const uint16_t thisNode = 00;

// Structure of our payload coming from router and end devices
struct Payload
{
  float aDCTemp; //temperature from onboard sensor
  bool batState; //bool to communicate battery power level, true is good and false means battery needs to be replaced
};

void setup() {
  Serial.begin(57600);
   while (!Serial) {
    ; // wait for serial port to connect. Sketch will stop here until open Serial Monitor
  }
  Serial.println("Coordinator is online.....");
  connectToWiFi(); //call function to connect to WiFi
  rtc.begin();  //init RTC
  static WiFiClient client; //create WiFi client object that will be used to get time and date from internet
  getServerTime(client); //Get the time from a server and set the RTC
  Serial.println("Got the time...");
  SPI.begin(); //start SPI communication
  radio.begin(); //start nRF24L01 communication layer
  network.begin(90, thisNode); //start nRF24L01 network layer, "90" is for channel
}

void loop() {
  network.update(); //update network and look for new nodes

  RF24NetworkHeader header; //create header variable to read data from node
  Payload payload; //create payload variable
  payload.aDCTemp = 0;
  // Any data on the network ready to read
  while ( network.available() )
  {
    // If so, grab it and print it out
    network.read(header,&payload,sizeof(payload)); //read data into payload struct
    Serial.print("The node this is from: ");
    Serial.println(header.from_node); //user header struct to print out node that data came from
    Serial.print("Temperature: ");
    Serial.print(payload.aDCTemp);
    Serial.print(" Battery status: ");
    Serial.println(payload.batState);
    Serial.println(getTimeDate());
    Serial.println();
  }
   
  if(payload.aDCTemp) { //if there is new data (non-zero) post it to cloud
    if (!postToPhant(header.from_node,getNodeData(payload.aDCTemp),getTimeDate(),payload.batState)) { Serial.println("failed to post data to Phant cloud....."); } //make call to send data to Phant cloud
    else { Serial.println("Sent data to cloud successfully...."); } 
  }
}

//This function builds a string of the time and date using the RTC
String getTimeDate() {
  return ((String)rtc.getHours() + ":" + (String)rtc.getMinutes() + ":" + (String)rtc.getSeconds() + " " + (String)rtc.getMonth() + "/" + (String)rtc.getDay() + "/" + (String)rtc.getYear());
}

//This function posts data to the Phant cloud, the input arguments are data for the fields we created on Phant
int postToPhant(int node, int temp, String timestamp, int batt)
{
  if(WiFi.status()!=WL_CONNECTED) { connectToWiFi(); } //if true we lost WiFi connection so attempt to reconnect
  // LED turns on when we enter, it'll go off when we successfully post.
  digitalWrite(LED_PIN, HIGH);
  
  // Declare an object from the Phant library - phant
  PhantMKR1K phant(PhantHost, PublicKey, PrivateKey);
  //These calls build the web communication
  phant.add("node", node); //specify field and data used in that field
  phant.add("temp", temp);
  phant.add("datatstamp", timestamp);
  phant.add("batt", batt);
  
  WiFiClient client; //Create client object to communicate with the phant server
 
  if (!client.connect(PhantHost, 80)) { //Attempt to connect to phant server using port 80
    // If we fail to connect, return 0.
    return 0;
  }

  //Send post to phant server
  client.print(phant.post()); //post datat to phant
  
  // if there are incoming bytes available from the server, read them and print them:
  while (client.available()) {
    String line = client.readStringUntil('\r');
    //Do something with data
  }
  client.stop(); //end the client
 // Before we exit, turn the LED off.
  digitalWrite(LED_PIN, LOW);
  
  return 1; // Return success
}

//this function takes float temp and converts it to int with one decimal point precision
//This is done because phant cloud cannot handle floats
int getNodeData(float data) {
  return data*10;
}

//This function connects to WiFi, while it is connecting the LED will go on
//If the LED is stuck on and not going off it means you have a connection problem
void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  digitalWrite(LED_PIN, HIGH); //turn on LED while we try to connect
  // check for the presence of the shield:
//  if (WiFi.status() == WL_NO_SHIELD) {
//    // don't continue:
//    Serial.println("No WiFi Shield detected so stop here....");
//    while (true);
//  }

  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
   status = WiFi.begin(ssid, pass); //call WiFi begin and wait for status to signal a connection was made
   while (status != WL_CONNECTED) {
    //loop until we connect
  }
  Serial.println("Connected to WiFi...");
  digitalWrite(LED_PIN, LOW); //turn on LED while we try to connect
}

//This function gets the time and date from a server of your choice on the internet
//The code in this function was leveraged from Francesco Potorti http://playground.arduino.cc/Code/Webclient
void getServerTime (Client &client)
{
  unsigned long time = 0;

  // Just choose any reasonably busy web server, the load is really low
  if (client.connect("g.cn", 80)) {
      // Make an HTTP 1.1 request which is missing a Host: header
      // compliant servers are required to answer with an error that includes
      // a Date: header.
      client.print(F("GET / HTTP/1.1 \r\n\r\n"));

      char buf[5];      // temporary buffer for characters
      client.setTimeout(5000);
      if (client.find((char *)"\r\nDate: ") // look for Date: header
    && client.readBytes(buf, 5) == 5) // discard
  {
  byte day = client.parseInt();    // get the date
  rtc.setDay(day); //set RTC date
  client.readBytes(buf, 1);    // discard this byte
  client.readBytes(buf, 3);    // get three letter month
  int year = client.parseInt();    // get the year
  year = year - 2000; //convert year to two digits
  rtc.setYear(year); //set year on RTC
  byte hour = client.parseInt();   //get hour
  rtc.setHours(hour); //set RTC hour
  byte minute = client.parseInt(); //get min
  rtc.setMinutes(minute); //set RTC min
  byte second = client.parseInt(); //get second
  rtc.setSeconds(second); //set RTC second

    uint8_t month; //varible to get month number from three letter abbrv
    switch (buf[0])
      {
      case 'F': month = 2; break; // Feb
      case 'S': month = 9; break; // Sep
      case 'O': month = 10; break; // Oct
      case 'N': month = 11; break; // Nov
      case 'D': month = 12; break; // Dec
      default:
        if (buf[0] == 'J' && buf[1] == 'a') month = 1;   // Jan
        else if (buf[0] == 'A' && buf[1] == 'p')  month = 4;    // Apr
        else switch (buf[2])
         {
         case 'r': month = 3; break; // Mar
         case 'y': month = 5; break; // May
         case 'n': month = 6; break; // Jun
         case 'l': month = 7; break; // Jul
         default: // add a default label here to avoid compiler warning
         case 'g': month = 8; break; // Aug
         }
      }
      rtc.setMonth(month); //set RTC month
  }
    }
  delay(10);
  client.flush();
  client.stop(); //kill client connection
}
