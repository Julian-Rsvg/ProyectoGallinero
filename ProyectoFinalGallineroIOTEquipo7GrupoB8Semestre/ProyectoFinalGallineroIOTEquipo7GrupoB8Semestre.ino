 
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <Servo.h> //Se define librería para las funciones de servo

// Set these to run example.
#define FIREBASE_HOST "proyectogallinero-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "MqTA4lxa3FIZIz7gWMS3aQHeD7yh8oOK0uVkK8jT"
//#define WIFI_SSID "ARRIS-8952"
//#define WIFI_PASSWORD "3C439D784D8D9711?"

/*#define WIFI_SSID "POCO"
#define WIFI_PASSWORD "123456789"*/

#define WIFI_SSID "HOME-E5C9"
#define WIFI_PASSWORD "9BD00A196B5016DF"

//:::::::::::::::::::::::::::::::variables para la temperatura:::::::::::::::::::::::::::::::::::::
int ventilador = 13;//para encender un ventilador si la temperatura es alta
int foco = 15;//para encender un foco si la temperatura es muy baja

//variables para la temperatura
float sensorvalor; //Declaramos variable
float rango; //Declaramos variable

//Agregamos unas variables para poder almacenar los datos y utilizarlo 
char dato_temperatura; //Declaramos variable de tipo cadena
char buffer [10];

//variable para configurar el pin del sensore de temperatura
int analogpin = A0;

//::::::::::::::::Variables para el dispensador de comida::::::::::::::::
Servo mot; //varaible para el servomotor(para el dispensador de alimento)

int Disp = 12;//para el dispensador de alimento

//definimos los pines que se van a utilizar para leer los datos
//para la distancia del agua
#define pEcho D1 //Declaramos el pin a utilizar para pEcho
#define pTrig D2 //Declaramos el pin a utilizar para pTrig

//Agregamos unas variablesa para poder almacenar los datos y utilizarlo 
char dato_ultrasonico; //Declaramos variable de tipo cadena

int duracion; //Declaramos variable
int ultrasonico = 0; //Declaramos variable

//:::::::variables para la bomba de agua
//definimos los pines que se van a utilizar para leer los datos
//para la distancia del agua
#define pEchod D3 //Declaramos el pin a utilizar para pEcho
#define pTrigd D4 //Declaramos el pin a utilizar para pTrig

//Agregamos unas variablesa para poder almacenar los datos y utilizarlo 
char dato_ultrasonicodos; //Declaramos variable de tipo cadena

int duracionsen; //Declaramos variable
int ultrasonicosen = 0; //Declaramos variable

//variables para la bomba de agua
int MtAgua = 14;//para la bomba de agua estara conectada en el D5

//variable con datos para comparar en firebase
String datoventi="1";
String datofoco="1";
String datodispen="1";
String datobomba="1";

void setup() {
  //::::: configuramos los bauidos del serial
  Serial.begin(9600);

  //::::::::configuracion de los pines del ventildaor y el foco
  //para el modulo de temperatura definnimos las salidas de los pines::::::::::::::
  pinMode(ventilador, OUTPUT);
  pinMode(foco, OUTPUT);

  //:::::Configuracion para los pines del modulo de sensor de comida
  pinMode(pEcho, INPUT); //Define a pEcho como entrada
  pinMode(pTrig, OUTPUT); //Define a pTrig como 

  //configuracion del pin de salida par activar el dispensador de comida
  pinMode(Disp, OUTPUT);

  //configuracion del servomotor
  mot.attach(12);//definimos el pin al que se va a conectar el servomotor
  mot.write(0);//definimos el angulo de inclinacion al iniciar
  delay(10);//definimos un tiempo

  //configuracion de los pines del sensor ultrasonico para la bomda de agua
  pinMode(pEchod, INPUT); //Define a pEcho como entrada
  pinMode(pTrigd, OUTPUT); //Define a pTrig como 

  //configuracion del pin de salida par activar la bomba de agua
  pinMode(MtAgua, OUTPUT);
  

  // conexcion a wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  //pinMode(2,OUTPUT);
}//termina la configuracion

int n = 0;

void loop() {
  //::::::::para obtener los datos de temperatura::::::::::::
  int temp = lectura_sensor(); //Declaramos constantes
  //:::::::::para obtener los datos del sensor de comida
  int dist = lectura_ultrasonico(); //Declaramos constantes
  //:::::::::para obtener los datos del sensor de agua
  int distan = lectura_ultrasonicodos(); //Declaramos constantes
  sprintf(buffer, "%d, %d, %d", temp, dist, distan);
  Serial.println(buffer); //Mandamos a pantalla el valor del buffer
  delay(10); //Definimos un tiempo

  //::::::Cindiciones para el dispensador de comida
  if(dist >= 8){
    //mostramos un mensaje en consola
    Serial.println("Nivel insuficiente de comida: ");
    Serial.println("Encendiendo dispensador de comida");
    //Servo_MotorOP();
    //Mandamos datos a firebase
    Firebase.setString("EstadoComida", "Insuficiente");
    // handle error
    //si hay un error mostrarlo en consola
    if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());  
      return;
    }
    //Cambiamos el estado del dispensador a 1 para activar el encendido
    Firebase.setString("Dispensador", "1");
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
  }else{
    //si es estable, imprimimos en pantalla
    Serial.println("Nivel estable:");
    //Servo_MotorCL();
    //se modifica el estado de la comida
    Firebase.setString("EstadoComida", "Suficiente");
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());  
      return;
    }
    //cambiamos el estado del dispensador a 0 apagado
    Firebase.setString("Dispensador", "0");
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
  }
//:::Condiciones para la temperatura
  if(temp >= 35){
    Serial.println("Temperatura elevada: ");
    Serial.println("Encendiendo ventilador");
    // set string value
    //Enviamos a firebase el estado de la temperatura 
    Firebase.setString("EstadoTemperatura", "Alta");
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());  
      return;
    }
    //se cambia el valor del ventilador a 1 Endendido
    Firebase.setString("Ventilador", "1");
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
  }///primera condicion
  //si la temperatura es baja se desactiva la secuencia
  else if(temp <= 25){
    Serial.println("Temperatura baja: ");
    Serial.println("Encendiendo foco");
    //FocoON();
    // set string value
    //se envia al firebase el estado de la temperatura 
    Firebase.setString("EstadoTemperatura", "Baja");
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());  
      return;
    }
    //se activa el encendido del foco
    Firebase.setString("Foco", "1");
  // handle error
    if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
    }
  }
  else{
    //si la temperatura es estable 
    Serial.println("Temperatura estable:");
    // set string value
    Firebase.setString("EstadoTemperatura", "Estable");
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());  
      return;
    }
    //se apaga el ventilador
    Firebase.setString("Ventilador", "0");
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
    }
    //se apaga el foco
    Firebase.setString("Foco", "0");
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
    }
  }

  //::::Comienza las instrucciones para el sensor del agua:::::::
  if(distan >= 8){
    Serial.println("Nivel insuficiente de agua");
    Serial.println("Encendiendo bomba de agua");
    Firebase.setString("EstadoAgua", "Insuficiente");
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());  
      return;
    }
    Firebase.setString("BombaAgua", "1");
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
  }else{
    Serial.println("Nivel estable:");
    Firebase.setString("EstadoAgua", "Estable");
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());  
      return;
    }
    Firebase.setString("BombaAgua", "0");
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
  }
  
  //get value
  //verifica el valor de la base de datos(firebase)
  //si el ventilador tiene un valor 1 se activa el ventilador
  if(Firebase.getString("Ventilador") == datoventi){
    VentiladorON();
  }else{
    //si el valor encontrado es 0 se desaactiva el ventilador
    VentiladorOFF();
  }

  //get value
  //verifica el dato de la etiqueta foco en firebase
  if(Firebase.getString("Foco") == datofoco){
    FocoON();//se activa el foco
  }else{
    Foco0FF();// se desactiva el foco
  }

  //get value
  //verifica el dato del dispensador
  if(Firebase.getString("Dispensador")== datodispen){
    Servo_MotorOP();// enciende dispensador de comida
  }else{
    Servo_MotorCL();//apaga el dispensador de comida
  }

  //get value
  if(Firebase.getString("BombaAgua")== datodispen){
    Motor_AguaON();
  }else{
     Motor_AguaOFF();
  }
  
  // set value
  Firebase.setFloat("Temperatura", temp);
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
  delay(50);
  
  // set value
  Firebase.setFloat("NivelComida", dist);
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
  delay(50);

  // set value
  Firebase.setFloat("NivelAgua", distan);
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
  delay(50);

  // append a new value to /logs
  String name = Firebase.pushInt("logs", n++);
  // handle error
  if (Firebase.failed()) {
      Serial.print("pushing /logs failed:");
      Serial.println(Firebase.error());  
      return;
  }
  Serial.print("pushed: /logs/");
  Serial.println(name);
  delay(50);
}//::::::::::termina la funcion de loop

//::::::::::::::::::::::para las funciones de los modulos::::::::::::::///

//:::::::modulo para el sensor de temperatura
void VentiladorON(){
  digitalWrite(ventilador, HIGH);
  delay(10);
}
void VentiladorOFF(){
  digitalWrite(ventilador, LOW);
  delay(10);
}

void FocoON(){
  digitalWrite(foco, HIGH);
  delay(10);
}
void Foco0FF(){
  digitalWrite(foco, LOW);
  delay(10);
}


//para obtener la temperatura
float lectura_sensor() { //Definimos variable
  sensorvalor = analogRead(analogpin); // Lee el valor del pin analógico especificado
  rango = (float (sensorvalor) / 1023) * 330.0;
  return rango; //Termina la función y devuelve un valor desde una función a la función que llama
}//termina la instrucion de la temperatura


//::::modulo de sensor de comida:::::::::::::
void Servo_MotorOP() {
  mot.write(180);
  delay(10);
}

void Servo_MotorCL() {
  mot.write(0);
  delay(10);
}


//para leer obtener la distancia
 float lectura_ultrasonico() //Declaramos una variable
{
  digitalWrite(pTrig, LOW); //Desactivamos el pin digital pTrig
  delayMicroseconds(2); //Definimos tiempo
  digitalWrite(pTrig, HIGH); //Activamos el ping digital pTrig
  delayMicroseconds(10); //Definimos tiempo
  digitalWrite(pTrig, LOW); //Desactivamos el pin digital pTrig
  duracion = pulseIn(pEcho, HIGH); // pulseIn si el valor es alto se espera a que pase a nivel high es como una funcion de tiempo
  ultrasonico = (duracion / 2) / 29;

  return ultrasonico; //Termina la función y devuelve un valor desde una función a la función que llama
}//termina la instrucioon del sensor de comida

//::::::::::Modulo para bomba de agua:::::::::::
void Motor_AguaON() {
  digitalWrite(MtAgua, HIGH);
  delay(10);
}

void Motor_AguaOFF() {
  digitalWrite(MtAgua, LOW);
  delay(10);
}

//para leer obtener la distancia
 float lectura_ultrasonicodos() //Declaramos una variable
{
  digitalWrite(pTrigd, LOW); //Desactivamos el pin digital pTrig
  delayMicroseconds(2); //Definimos tiempo
  digitalWrite(pTrigd, HIGH); //Activamos el ping digital pTrig
  delayMicroseconds(10); //Definimos tiempo
  digitalWrite(pTrigd, LOW); //Desactivamos el pin digital pTrig
  duracionsen = pulseIn(pEchod, HIGH); // pulseIn si el valor es alto se espera a que pase a nivel high es como una funcion de tiempo
  ultrasonicosen = (duracionsen / 2) / 29;

  return ultrasonicosen; //Termina la función y devuelve un valor desde una función a la función que llama
}
