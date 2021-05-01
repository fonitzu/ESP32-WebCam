//#include <Arduino.h>
/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

// Load Wi-Fi library
#include <WiFi.h>
#include <WebServer.h>

// Replace with your network credentials
const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";

WebServer server( 80 );

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

String Ssid;
String Psk;

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

void loop( void )
{
	server.handleClient();
}
