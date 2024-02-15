// Does not function 
// The Led on the module lights but the data doesn't change

#define digitalPin 23 
#define analogPin 22 

void setup ()
{
  pinMode (digitalPin, INPUT); 
  pinMode(analogPin, INPUT); 
  Serial.begin(9600);
}
void loop ()
{
  // Read the digital interface
  Serial.println(digitalRead(digitalPin));
  
  // Read the analog interface
  Serial.println(analogRead(analogPin)); 
  delay(100);
}