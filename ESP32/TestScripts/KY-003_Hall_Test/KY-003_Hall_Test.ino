#define sensor 23

void setup()
{
  Serial.begin(9600);
  //set sensor pin as input
	pinMode(sensor, INPUT); 
}
void loop()
{
	//Read the sensor
	Serial.println(digitalRead(sensor));
}







