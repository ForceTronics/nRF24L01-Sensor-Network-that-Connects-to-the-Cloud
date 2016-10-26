/*10/24/16 This code was written for the YouTube video "Creating a Sensor Network that Connects to the Cloud Part 1"
 * on the ForceTronics channel. This code is public domain and free to others to use and modify at your own risk
 */
#include <RF24.h> //nRF24L01 library: https://github.com/TMRh20/RF24
#include <RF24NetworkNoSleep.h> //modified verision of the RF24Network library to work on non-AVR platform
#include <SPI.h> 
#include <WiFi101.h> //used for WiFi on MKR1000 and WiFi shield

char ssid[] = "YourNetworkName"; //  your network SSID (name)
char pass[] = "YourNetworkPassword";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS; //set status variable

//global variables for holding sensor data
int tempNode1 = 0;
int tempNode2 = 0;
int tempNode3 = 0;

//define some constant variables for pins and node number
const int LED_PIN = 6; // MKR1000's built-in LED

//define areas for phant cloud address and security keys
const char PhantHost[] = "data.sparkfun.com";
const char PublicKey[] = "YourPublicKey";
const char PrivateKey[] = "YourPrivateKey";

//these variables were leveraged from Phant library for ESP8266, created by Sparkfun
String _pub;
String _prv;
String _host;
String _params;
static const char HEADER_POST_URL1[] PROGMEM = "POST /input/";
static const char HEADER_POST_URL2[] PROGMEM = ".txt HTTP/1.1\n";
static const char HEADER_PHANT_PRV_KEY[] PROGMEM = "Phant-Private-Key: ";
static const char HEADER_CONNECTION_CLOSE[] PROGMEM = "Connection: close\n";
static const char HEADER_CONTENT_TYPE[] PROGMEM = "Content-Type: application/x-www-form-urlencoded\n";
static const char HEADER_CONTENT_LENGTH[] PROGMEM = "Content-Length: ";

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

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    // don't continue:
    Serial.println("No WiFi Shield detected so stop here....");
    while (true);
  }

  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
   status = WiFi.begin(ssid, pass);

   while (status != WL_CONNECTED) {
    //loop until we connect
  }
  Serial.println("Connected to WiFi...");
 
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
    network.read(header,&payload,sizeof(payload));
    Serial.print("The node this is from: ");
    Serial.println(header.from_node);
    Serial.print("Temperature: ");
    Serial.print(payload.aDCTemp);
    Serial.print(" Battery status: ");
    Serial.println(payload.batState);
    Serial.println();
    getNodeData(header.from_node, payload.aDCTemp); //store data in global variable for each node
  }
   
  if(payload.aDCTemp) { //if there is new data (non-zero) post it to cloud
    if (!postToPhant()) { Serial.println("failed to post data to Phant cloud....."); }
    else { Serial.println("Sent data to cloud successfully...."); }
  }
}

//This function posts data to the Phant cloud
int postToPhant()
{
  // LED turns on when we enter, it'll go off when we successfully post.
  digitalWrite(LED_PIN, HIGH);
  
  // Declare an object from the Phant library - phant
  phant(PhantHost, PublicKey, PrivateKey);
  //These calls build the web communication
  phantAdd("mnode1", tempNode1); //specify field and data used in that field
  phantAdd("mnode2", tempNode2);
  phantAdd("mnode3", tempNode3);
  
  WiFiClient client; //Create client object to communicate with the phant server
 
  if (!client.connect(PhantHost, 80)) { //Attempt to connect to phant server using port 80
    // If we fail to connect, return 0.
    return 0;
  }

  //Serial.println(phantPost()); //uncomment this if you want to see text of communication with phant server
  //Send post to phant server
  client.print(phantPost()); //post datat to phant
  
  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    String line = client.readStringUntil('\r');
    //Do something with data
  }
  client.stop(); //end the client
 // Before we exit, turn the LED off.
  digitalWrite(LED_PIN, LOW);
  
  return 1; // Return success
}

//this function gets incoming data from the network and stores it in the right global variable
void getNodeData(uint16_t node, float data) {
  if(node == 1) tempNode1 = data*10; //data is multiplied by 10 because phant cannot handle decimals
  else if(node == 2) tempNode2 = data*10;
  else tempNode3 = data*10;
}

//This is from phant library, initializes variables
void phant(String host, String publicKey, String privateKey) {
  _host = host;
  _pub = publicKey;
  _prv = privateKey;
  _params = "";
}

//From phant library, builds string of field and data
void phantAdd(String field, int data) {

  _params += "&" + field + "=" + String(data);

}

//From phant library, builds the string used to post data to phant over web services
String phantPost() {

  String params = _params.substring(1);
  String result;
  //String result = "POST /input/" + _pub + ".txt HTTP/1.1\n";
  for (int i=0; i<strlen(HEADER_POST_URL1); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_POST_URL1 + i);
  }
  result += _pub;
  for (int i=0; i<strlen(HEADER_POST_URL2); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_POST_URL2 + i);
  }
  result += "Host: " + _host + "\n";
  //result += "Phant-Private-Key: " + _prv + "\n";
  for (int i=0; i<strlen(HEADER_PHANT_PRV_KEY); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_PHANT_PRV_KEY + i);
  }
  result += _prv + '\n';
  //result += "Connection: close\n";
  for (int i=0; i<strlen(HEADER_CONNECTION_CLOSE); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_CONNECTION_CLOSE + i);
  }
  //result += "Content-Type: application/x-www-form-urlencoded\n";
  for (int i=0; i<strlen(HEADER_CONTENT_TYPE); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_CONTENT_TYPE + i);
  }  
  //result += "Content-Length: " + String(params.length()) + "\n\n";
  for (int i=0; i<strlen(HEADER_CONTENT_LENGTH); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_CONTENT_LENGTH + i);
  } 
  result += String(params.length()) + "\n\n";
  result += params;

  _params = "";
  return result;
}

