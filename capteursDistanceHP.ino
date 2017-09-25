    //**********************************************************
    //  Capteur de distance à ultrasons HC-SR04 avec Arduino Uno
    //  TipTopBoards.com  06 01 2014  Rolland
    //  Càabler +5V et GND du détecteur sur l'arduino
    //
    // Trig -> pin 13
    // Echo -> pin 12
    // Led verte + 470R  -> pin 10
    // Led rouge + 470R  -> pin 11
    // Les leds s'allument selon la distance trop près / trop loin
    //**********************************************************
    #define trigPin 13  //Trig Ultrasons (sortie)
    #define echoPin 12  //Echo Ultrasons (entrée)
    #define ledVerte 11      //Led verte
    #define ledRouge 10     //Led rouge

int speakerPin = 9;

int length = 8; // le nombre de notes

char names[] = { 'D', 'R', 'M', 'F', 'S', 'L', 'I', 'd' };
int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };

  
char notes[] = "DRMFSLId"; // un espace = un repos
int beats[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4 };
int tempo = 300;

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

void playNote(char note, int duration)
{


  // jouer la tonalite correspondant au nom de la note
  for (int i = 0; i < 8; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}

void playFreq(int distance)
{
     int tone= 1000 + (distance*10);
     //Serial.println("tone = ");
     //Serial.println(tone);

     playTone(tone,tempo);
}


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


    void setup() {

      Serial.begin (9600); 
      pinMode(trigPin, OUTPUT);  //Trig est une sortie
      pinMode(echoPin, INPUT);   //Echo est le retour, en entrée
      pinMode(ledVerte, OUTPUT);
      pinMode(ledRouge, OUTPUT);

      pinMode(speakerPin, OUTPUT);
    }

    void loop() {
      long duration, distance;
      
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
         
      if (distance < 10 && distance > 0) {  // Distance trop près
        digitalWrite(ledVerte,HIGH); //Vert éclairé seulement
        digitalWrite(ledRouge,LOW);    }
      else {   //Trop loin
        digitalWrite(ledVerte,LOW);
        digitalWrite(ledRouge,HIGH);
      }
      //Hors de portée (duration)
      if ( distance <= 0){ 
        //Serial.print(duration);
        Serial.println("Hors de portee");
      }
      else {
        playFreq(distance);

        Serial.print(distance);
        Serial.print(" cm ");
        //Serial.print(duration);
        //Serial.println(" ms");
      }
      delay(100);  //0.1 sec entre deux mesures

      // bluetooth
      // listen for the data
      if ( Serial.available() > 0 ) {
      // read a numbers from serial port
      int count = Serial.parseInt();
    
      // print out the received number
      if (count > 0) {
        Serial.print("You have input: ");
        Serial.println(String(count));
        // blink the LED
        // blinkLED(count);
      } // if (count > 0)
      }// if ( Serial.available() > 0 )
    }

