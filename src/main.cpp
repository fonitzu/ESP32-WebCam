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
#include "esp_camera.h"

#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"

// Access point name
const char *ssid = "ESP32-Access-Point";
// Access point password
const char *password = "123456789";

WebServer server(80);

/*
	Web page (root), used to set the network name to connect to and the password.

	POST is used to send data back to ESP32-CAM (network name and password) when pressing "Submit".
	With "Connect", the Access point is turned off and the ESP32-CAM connects to the given
	network using given password.
*/
const char *html =
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
	for( int i = 0; i < server.args(); ++i )
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


void HandleImage( void )
{
	camera_fb_t* fb = esp_camera_fb_get();
	if( fb == nullptr )
	{
		return;
	}

	Serial.print( "New image [" );
	Serial.print( fb->width );
	Serial.print( " x " );
	Serial.print( fb->height );
	Serial.print( " ]." );

	//replace this with your own function
	//display_image(fb->width, fb->height, fb->pixformat, fb->buf, fb->len);

	server.send_P( 200, "image/jpeg", (const char*)fb->buf, fb->len );
	//return the frame buffer back to be reused
	esp_camera_fb_return( fb );
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

void SetupCamera(void)
{
	camera_config_t config;
	config.ledc_channel = LEDC_CHANNEL_0;
	config.ledc_timer = LEDC_TIMER_0;
	config.pin_d0 = Y2_GPIO_NUM;
	config.pin_d1 = Y3_GPIO_NUM;
	config.pin_d2 = Y4_GPIO_NUM;
	config.pin_d3 = Y5_GPIO_NUM;
	config.pin_d4 = Y6_GPIO_NUM;
	config.pin_d5 = Y7_GPIO_NUM;
	config.pin_d6 = Y8_GPIO_NUM;
	config.pin_d7 = Y9_GPIO_NUM;
	config.pin_xclk = XCLK_GPIO_NUM;
	config.pin_pclk = PCLK_GPIO_NUM;
	config.pin_vsync = VSYNC_GPIO_NUM;
	config.pin_href = HREF_GPIO_NUM;
	config.pin_sscb_sda = SIOD_GPIO_NUM;
	config.pin_sscb_scl = SIOC_GPIO_NUM;
	config.pin_pwdn = PWDN_GPIO_NUM;
	config.pin_reset = RESET_GPIO_NUM;
	config.xclk_freq_hz = 20000000;
	config.pixel_format = PIXFORMAT_JPEG;
	//init with high specs to pre-allocate larger buffers
	if( psramFound() )
	{
		config.frame_size = FRAMESIZE_UXGA;
		config.jpeg_quality = 10;
		config.fb_count = 2;
	}
	else
	{
		config.frame_size = FRAMESIZE_SVGA;
		config.jpeg_quality = 12;
		config.fb_count = 1;
	}

	// camera init
	esp_err_t err = esp_camera_init( &config );
	if( err != ESP_OK )
	{
		Serial.printf( "Camera init failed with error 0x%x", err );
		return;
	}

/*
	sensor_t* s = esp_camera_sensor_get();
	s->set_framesize( s, FRAMESIZE_UXGA );*/
}

// Start the Access point and wait for network configuration data (name and password)
void setup()
{
	delay( 5000 );
	Serial.begin( 115200 );

	Serial.println( "ESP32-CAM Starting..." );

	SetupCamera();

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
	server.on( "/image.jpg", HandleImage );
	server.begin();
	Serial.println( "HTTP server started" );
}

// Run the web server
void loop( void )
{
	server.handleClient();
}
