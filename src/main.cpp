//#include <Arduino.h>
/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

/*
	Open an access point to be able to configure the network name and credentials
	for the network which will connect to.

	After connecting to "ESP32-Access-Point", open 192.168.4.1 to enter
	the network name and password.

	The ESP32-CAM will turn off the access point and connect to the network
	using given password.
*/

// Load Wi-Fi library
#include <WiFi.h>
#include <WebServer.h>

// Access point name
const char* ssid     = "ESP32-Access-Point";
// Access point password
const char* password = "123456789";

WebServer server( 80 );

/*
	Web page (root), used to set the network name to connect to and the password.

	POST is used to send data back to ESP32-CAM (network name and password) when pressing "Submit".
	With "Connect", the Access point is turned off and the ESP32-CAM connects to the given
	network using given password.
*/
const char * html =            
"<!DOCTYPE html>\
<html>\
	<head>\
		<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
		<style>\
			html \
			{ \
				font-family: Helvetica; \
				display: inline-block; \
				margin: 0px auto;\
				text-align: center;\
			}\
		</style>\
	</head>\
	<body>\
		<h1>ESP32 Web Server</h1>\
		<p>Current SSID: %s1</p>\
		<p>Current PSK: %s2</p>\
		<hr/>\
		<p>Enter new SSID/PSK</p>\
		<form action=\"/\" method=\"post\">\
			<label for=\"SSID\">SSID:</label>\
			<input type=\"text\" id=\"SSID\" name=\"SSID\" value=\"%s1\"><br><br>\
			<label for=\"PSK\">PSK:</label>\
			<input type=\"password\" id=\"PSK\" name=\"PSK\" value=\"%s2\"><br><br>\
			<input type=\"submit\" value=\"Submit\">\
		</form>\
		<hr/>\
		<form action=\"/connect\">\
			<input type=\"submit\" value=\"Connect\"/>\
		</form>\
	</body>\
</html>";

// Network name to connect to
String Ssid;
// Password for the network to connect to
String Psk;

// Retrieve network name and password for the network to connect to
void HandleRoot( void )
{
	String root_page( html );

	Serial.println( "Parsing web page: " );
	Serial.printf( "POST arguments: %d\r\n", server.args() );
	for( int i = 0; i < server.args(); ++ i )
	{
		Serial.print( server.argName( i ) );
		Serial.print( ": " );
		Serial.println( server.arg( i ) );
	}

	if( server.hasArg( "SSID" ) )
	{
		Ssid = server.arg( "SSID" );
	}

	if( server.hasArg( "PSK" ) )
	{
		Psk = server.arg( "PSK" );
	}
	
	root_page.replace( "%s1", Ssid );
	root_page.replace( "%s2", Psk );
	server.send( 200, "text/html", root_page );
}

// Turn the Access point off and connect to the given network
void HandleConnect( void )
{
	Serial.println( "Disconnecting.." );
	server.close();
	WiFi.softAPdisconnect();
	WiFi.mode( WIFI_STA );
	Serial.print( "Connecting to " );
	Serial.println( Ssid );
	wl_status_t result = WiFi.begin( Ssid.c_str(), Psk.c_str() );
	Serial.println( "Connected." );
	server.begin();
}

// Start the Access point and wait for network configuration data (name and password)
void setup()
{
  Serial.begin( 115200 );
  // Connect to Wi-Fi network with SSID and password
  Serial.println( "Web Server " );
  Serial.print( "Setting AP (Access Point)…" );
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP( ssid, password );

  IPAddress IP = WiFi.softAPIP();
  Serial.print( "AP IP address: " );
  Serial.println( IP );

  server.on( "/", HandleRoot );
  server.on( "/connect", HandleConnect );
  server.begin();
  Serial.println( "HTTP server started" );
}

// Run the web server
void loop( void )
{
	server.handleClient();
}
