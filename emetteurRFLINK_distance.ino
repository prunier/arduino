//**********************************************************
//  Capteur de distance à ultrasons HC-SR04 avec Arduino nano 
//  avec émetteur RFLINK type Oregon
//
//  Capteur de distance  Trig -> pin 3 (pas 13)
//  Capteur de distance  Echo -> pin 2 (pas 12)
//  Capteur de distance  Haut-Parleur -> pin 9 

//  Emetteur  RFLINK     data  -> pin  8
//**********************************************************

#include "LowPower.h"
#include <OneWire.h>

//**********************************************************
//  Variables Globales
//**********************************************************

// ---------------------------------------
//  partie Capteur de distance à ultrasons 
// ---------------------------------------

#define trigPin 3  //Trig Ultrasons (sortie)
#define echoPin 2  //Echo Ultrasons (entrée)

int speakerPin = 9;

int length = 8; // le nombre de notes

char names[] = { 'D', 'R', 'M', 'F', 'S', 'L', 'I', 'd' };
int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };

  
char notes[] = "DRMFSLId"; // un espace = un repos
int beats[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4 };
int tempo = 300;

// ---------------------------------------
//  Partie  émetteur RFLINK type Oregon
// ---------------------------------------

#define DS18B20 0x28     // Adresse 1-Wire du DS18B20
#define BROCHE_ONEWIRE 10 // Broche utilisÃ©e pour le bus 1-Wire
 
OneWire ds(BROCHE_ONEWIRE); // CrÃ©ation de l'objet OneWire ds
 
#define THN132N
 
const byte TX_PIN = 8;
 
const unsigned long TIME = 512;
const unsigned long TWOTIME = TIME*2;
 
#define SEND_HIGH() digitalWrite(TX_PIN, HIGH)
#define SEND_LOW() digitalWrite(TX_PIN, LOW)
 
// Buffer for Oregon message
#ifdef THN132N
  byte OregonMessageBuffer[8];
#else
  byte OregonMessageBuffer[9];
#endif

    
//**********************************************************
//  partie Capteur de distance à ultrasons 
//  TipTopBoards.com  06 01 2014  Rolland
//  Càabler +5V et GND du détecteur sur l'arduino
//
//  Capteur de distance  Trig -> pin 3 (pas 13)
//  Capteur de distance  Echo -> pin 2 (pas 12)
//
//**********************************************************

    

/**
 * \brief    jouer un ton
 * \param    tone    code ton
 * \param    duration    durée milliseconde
 */
void playTone(int tone, int duration)
{
  for (long i = 0; i < duration * 1000L; i += tone * 2)
  {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
}

/**
 * \brief    jouer une note
 * \param    tone    code note
 * \param    duration    durée milliseconde
 */
void playNote(char note, int duration)
{


  // jouer la tonalite correspondant au nom de la note
  for (int i = 0; i < 8; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}

/**
 * \brief    jouer une fréquence selon la distance
 * \param    distance    distance en cm
 */
void playFreq(int distance)
{
     int tone= 1000 + (distance*10);
     //Serial.println("tone = ");
     //Serial.println(tone);

     playTone(tone,tempo);
}

/**
 * \brief    jouer une mélodie 
*/
void playMelody()
{
  for (int i = 0; i < length; i++)
  {
    if (notes[i] == ' ')
    {
      delay(beats[i] * tempo); // repos
    }
    else
    {
      playNote(notes[i], beats[i] * tempo);
    }

    // pause entre les notes
    delay(tempo / 2); 
  }
}


//**********************************************************
//  partie émetteur RFLINK 
//  Emetteur  RFLINK     data  -> pin 10 ?
//
//***********************************************************


 
/**
 * \brief    Send logical "0" over RF
 * \details  azero bit be represented by an off-to-on transition
 * \         of the RF signal at the middle of a clock period.
 * \         Remenber, the Oregon v2.1 protocol add an inverted bit first 
 */
inline void sendZero(void) 
{
  SEND_HIGH();
  delayMicroseconds(TIME);
  SEND_LOW();
  delayMicroseconds(TWOTIME);
  SEND_HIGH();
  delayMicroseconds(TIME);
}
 
/**
 * \brief    Send logical "1" over RF
 * \details  a one bit be represented by an on-to-off transition
 * \         of the RF signal at the middle of a clock period.
 * \         Remenber, the Oregon v2.1 protocol add an inverted bit first 
 */
inline void sendOne(void) 
{
   SEND_LOW();
   delayMicroseconds(TIME);
   SEND_HIGH();
   delayMicroseconds(TWOTIME);
   SEND_LOW();
   delayMicroseconds(TIME);
}
 
/**
* Send a bits quarter (4 bits = MSB from 8 bits value) over RF
*
* @param data Source data to process and sent
*/
 
/**
 * \brief    Send a bits quarter (4 bits = MSB from 8 bits value) over RF
 * \param    data   Data to send
 */
inline void sendQuarterMSB(const byte data) 
{
  (bitRead(data, 4)) ? sendOne() : sendZero();
  (bitRead(data, 5)) ? sendOne() : sendZero();
  (bitRead(data, 6)) ? sendOne() : sendZero();
  (bitRead(data, 7)) ? sendOne() : sendZero();
}
 
/**
 * \brief    Send a bits quarter (4 bits = LSB from 8 bits value) over RF
 * \param    data   Data to send
 */
inline void sendQuarterLSB(const byte data) 
{
  (bitRead(data, 0)) ? sendOne() : sendZero();
  (bitRead(data, 1)) ? sendOne() : sendZero();
  (bitRead(data, 2)) ? sendOne() : sendZero();
  (bitRead(data, 3)) ? sendOne() : sendZero();
}
 
/******************************************************************/
/******************************************************************/
/******************************************************************/
 
/**
 * \brief    Send a buffer over RF
 * \param    data   Data to send
 * \param    size   size of data to send
 */
void sendData(byte *data, byte size)
{
  for(byte i = 0; i < size; ++i)
  {
    sendQuarterLSB(data[i]);
    sendQuarterMSB(data[i]);
  }
}
 
/**
 * \brief    Send an Oregon message
 * \param    data   The Oregon message
 */
void sendOregon(byte *data, byte size)
{
    sendPreamble();
    //sendSync();
    sendData(data, size);
    sendPostamble();
}
 
/**
 * \brief    Send preamble
 * \details  The preamble consists of 16 "1" bits
 */
inline void sendPreamble(void)
{
  byte PREAMBLE[]={0xFF,0xFF};
  sendData(PREAMBLE, 2);
}
 
/**
 * \brief    Send postamble
 * \details  The postamble consists of 8 "0" bits
 */
inline void sendPostamble(void)
{
#ifdef THN132N
  sendQuarterLSB(0x00);
#else
  byte POSTAMBLE[]={0x00};
  sendData(POSTAMBLE, 1);  
#endif
}
 
/**
 * \brief    Send sync nibble
 * \details  The sync is 0xA. It is not use in this version since the sync nibble
 * \         is include in the Oregon message to send.
 */
inline void sendSync(void)
{
  sendQuarterLSB(0xA);
}
 
/******************************************************************/
/******************************************************************/
/******************************************************************/
 
/**
 * \brief    Set the sensor type
 * \param    data       Oregon message
 * \param    type       Sensor type
 */
inline void setType(byte *data, byte* type) 
{
  data[0] = type[0];
  data[1] = type[1];
}
 
/**
 * \brief    Set the sensor channel
 * \param    data       Oregon message
 * \param    channel    Sensor channel (0x10, 0x20, 0x30)
 */
inline void setChannel(byte *data, byte channel) 
{
    data[2] = channel;
}
 
/**
 * \brief    Set the sensor ID
 * \param    data       Oregon message
 * \param    ID         Sensor unique ID
 */
inline void setId(byte *data, byte ID) 
{
  data[3] = ID;
}
 
/**
 * \brief    Set the sensor battery level
 * \param    data       Oregon message
 * \param    level      Battery level (0 = low, 1 = high)
 */
void setBatteryLevel(byte *data, byte level)
{
  if(!level) data[4] = 0x0C;
  else data[4] = 0x00;
}
 
/**
 * \brief    Set the sensor temperature
 * \param    data       Oregon message
 * \param    temp       the temperature
 */
void setTemperature(byte *data, float temp) 
{
  // Set temperature sign
  if(temp < 0)
  {
    data[6] = 0x08;
    temp *= -1;  
  }
  else
  {
    data[6] = 0x00;
  }
 
  // Determine decimal and float part
  int tempInt = (int)temp;
  int td = (int)(tempInt / 10);
  int tf = (int)round((float)((float)tempInt/10 - (float)td) * 10);
 
  int tempFloat =  (int)round((float)(temp - (float)tempInt) * 10);
 
  // Set temperature decimal part
  data[5] = (td << 4);
  data[5] |= tf;
 
  // Set temperature float part
  data[4] |= (tempFloat << 4);
}
 
/**
 * \brief    Set the sensor humidity
 * \param    data       Oregon message
 * \param    hum        the humidity
 */
void setHumidity(byte* data, byte hum)
{
    data[7] = (hum/10);
    data[6] |= (hum - data[7]*10) << 4;
}
 
/**
 * \brief    Sum data for checksum
 * \param    count      number of bit to sum
 * \param    data       Oregon message
 */
int Sum(byte count, const byte* data)
{
  int s = 0;
 
  for(byte i = 0; i<count;i++)
  {
    s += (data[i]&0xF0) >> 4;
    s += (data[i]&0xF);
  }
 
  if(int(count) != count)
    s += (data[count]&0xF0) >> 4;
 
  return s;
}
 
/**
 * \brief    Calculate checksum
 * \param    data       Oregon message
 */
void calculateAndSetChecksum(byte* data)
{
#ifdef THN132N
    int s = ((Sum(6, data) + (data[6]&0xF) - 0xa) & 0xff);
 
    data[6] |=  (s&0x0F) << 4;     data[7] =  (s&0xF0) >> 4;
#else
    data[8] = ((Sum(8, data) - 0xa) & 0xFF);
#endif
}
 
/******************************************************************/
/******************************************************************/

// Fonction rÃ©cupÃ©rant la tempÃ©rature depuis le DS18B20
// Retourne true si tout va bien, ou false en cas d'erreur
boolean getTemperature(float *temp){
  byte data[9], addr[8];
  // data : DonnÃ©es lues depuis le scratchpad
  // addr : adresse du module 1-Wire dÃ©tectÃ©
 
  
  if (!ds.search(addr)) { // Recherche un module 1-Wire
    ds.reset_search();    // RÃ©initialise la recherche de module
    return false;         // Retourne une erreur
  }
   
  if (OneWire::crc8(addr, 7) != addr[7]) // VÃ©rifie que l'adresse a Ã©tÃ© correctement reÃ§ue
    return false;                        // Si le message est corrompu on retourne une erreur

  if (addr[0] != DS18B20) // VÃ©rifie qu'il s'agit bien d'un DS18B20
    return false;         // Si ce n'est pas le cas on retourne une erreur
 
  ds.reset();             // On reset le bus 1-Wire
  ds.select(addr);        // On sÃ©lectionne le DS18B20
   
  ds.write(0x44, 1);      // On lance une prise de mesure de tempÃ©rature
  delay(1000);             // Et on attend la fin de la mesure
   
  ds.reset();             // On reset le bus 1-Wire
  ds.select(addr);        // On sÃ©lectionne le DS18B20
  ds.write(0xBE);         // On envoie une demande de lecture du scratchpad
 
  for (byte i = 0; i < 9; i++) // On lit le scratchpad
    data[i] = ds.read();       // Et on stock les octets reÃ§us
   
  // Calcul de la tempÃ©rature en degrÃ© Celsius
  *temp = ((data[1] << 8) | data[0]) * 0.0625; 
   
  // Pas d'erreur
  return true;
}


 //**********************************************************
//  setup
//**********************************************************


void setup() {

// ---------------------------------------
//  partie Capteur de distance à ultrasons 
// --------------------------------------- 

  Serial.begin (9600); 
  pinMode(trigPin, OUTPUT);  //Trig est une sortie
  pinMode(echoPin, INPUT);   //Echo est le retour, en entrée
  //pinMode(ledVerte, OUTPUT);
  //pinMode(ledRouge, OUTPUT);
  Serial.println("\nCapteur de distance à ultrasons");


  pinMode(speakerPin, OUTPUT);

// ---------------------------------------
//  Partie  émetteur RFLINK type Oregon
// ---------------------------------------

  digitalWrite(13,LOW);
  pinMode(TX_PIN, OUTPUT);
 
  //Serial.begin(38400);
  Serial.println("\n[Oregon V2.1 encoder]");
 
  SEND_LOW();  
 
#ifdef THN132N  
  // Create the Oregon message for a temperature only sensor (TNHN132N)
  byte ID[] = {0xEA,0x4C};
#else
  // Create the Oregon message for a temperature/humidity sensor (THGR2228N)
  byte ID[] = {0x1A,0x2D};
#endif  
 
  setType(OregonMessageBuffer, ID);
  setChannel(OregonMessageBuffer, 0x20);
  setId(OregonMessageBuffer, 0xBB);

}

//**********************************************************
//  loop 
//**********************************************************
    
void loop() {

// ---------------------------------------
//  partie Capteur de distance à ultrasons 
// ---------------------------------------

  long duration;
  float distance;
  
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); //Trig déclenché 10ms sur HIGH
  digitalWrite(trigPin, LOW);
 
  // Calcul de l'écho
  duration = pulseIn(echoPin, HIGH);
  // Distance proportionnelle à la durée de sortie
  distance = duration*340/(2*10000);  //Vitesse du son théorique
  //Serial.print(distance);
  //Serial.print(" cm ");
     
  //Hors de portée (duration)
  if ( distance <= 0){ 
    //Serial.print(duration);
    Serial.println("Hors de portee");
  } else {
    playFreq(distance);

    Serial.print(distance);
    Serial.print(" cm ");

    // RFLINK 
    setBatteryLevel(OregonMessageBuffer, 0); // 0 : low, 1 : high
    setTemperature(OregonMessageBuffer, distance);


    // Calculate the checksum
    calculateAndSetChecksum(OregonMessageBuffer);
 
    // Show the Oregon Message
    for (byte i = 0; i < sizeof(OregonMessageBuffer); ++i)   {
      Serial.print(OregonMessageBuffer[i] >> 4, HEX);
      Serial.print(OregonMessageBuffer[i] & 0x0F, HEX);    
     }

    // Send the Message over RF
    sendOregon(OregonMessageBuffer, sizeof(OregonMessageBuffer));
    // Send a "pause"
    SEND_LOW();
    delayMicroseconds(TWOTIME*8);
    // Send a copie of the first message. The v2.1 protocol send the
    // message two time 
    sendOregon(OregonMessageBuffer, sizeof(OregonMessageBuffer));
   
    // Wait for 30 seconds before send a new message 
    SEND_LOW();
    
//    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);  
//    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);  
//    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);  
//    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);  


     // capteur distance
     delay(100);  //0.1 sec entre deux mesures

  } // if ( distance <= 0){ 

 
} //loop


