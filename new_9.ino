

#include <DallasTemperature.h>
#include <OneWire.h>
#include <math.h> 
#include <stdlib.h> 
//#include <ArduinoJson.h>
#include "FS.h"

//#include <LiquidCrystal.h>

//ардуино 181, либы схоронил
//#include <Wire.h>
//#include <Adafruit_BMP085.h>
//

//#include <Adafruit_Sensor.h>
//#include <DHT.h>
//#include <LiquidCrystal_I2C.h>
//LiquidCrystal_I2C lcd(0x27, 16, 2);

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 

#include <WiFiClient.h>

#define debug true // вывод отладочных сообщений
#define postingInterval  300000 // интервал между отправками данных в миллисекундах (5 минут)

//#define DHTPIN   2   // dht на gpio02

// Uncomment the type of sensor in use:
//#define DHTTYPE           DHT11     // DHT 11 
//#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)
// Lib instantiate
//DHT dht(DHTPIN, DHTTYPE);

//Adafruit_BMP085 bmp;

/**/unsigned long lastConnectionTime = 0;// время последней передачи данных
unsigned long lastTime=0;
/**/String Hostname; //имя железки - выглядит как ESPAABBCCDDEEFF т.е. ESP+mac адрес.

signed short tInputK=0; //температура на входе в котёл
signed short tInputC=0; //температура на входе в смеситель
signed short tOutputK=0; //температура на выходе из котла
signed short tOutputC=0; //температура на выходе из смесителя
signed short tOut=0; //температура на улице
signed short tIn=0; //температура в помещении

unsigned char tBreakOffK = 80; //температура на выходе из котла для обязательного отключения вентелятора
unsigned char tBreakOffIn = 18; //температура в помещении для отключения вентелятора
unsigned char tDeltaKCritical = 18; //дельта температура от tBreakOffK на выходе из котла для обязательного включения вентелятора во имя обязательной температуры теплоносителя
unsigned char tDeltaKCritical_economy = 25; //дельта температура от tBreakOffK на выходе из котла для обязательного включения вентелятора во имя обязательной температуры теплоносителя в эконом режиме(4)
unsigned char tDeltaIn = 4; //дельта температура в помещении для регулировки скорости вращения от 100% до 0%
unsigned char tOutCritical=21; //температура на улице для отключения вентелятора
unsigned char ryj=12;//ручной коэффициент скорости

signed short schet = 0;
signed short schetPer = 7;
byte indexforindex = 0;
unsigned short tmp1 = 0;
float kof;
bool razgon = true;
bool razgon_tmp = false;
bool negative = true;

//залп
bool zalp = true;
bool zalp_per = false;
unsigned char zalp_c = 1;
//конец залп

byte global = 0;

String str_debug;
unsigned char rew = 1; //режим. 1 соответствует авто. 0 только поддержка оптиума для поддержания котла в режиме не конденсата на его стенах
String message1;
const String tr_td="<tr><td>";
const String _td_td="</td><td>";
const String _td__tr="</td><tr>";

  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  
//signed char tOutputK; //температура на выходе из котла
//signed char tOutputK; //температура на выходе из котла
//signed char tOutputK; //температура на выходе из котла
//signed char tInputK; //температура на входе в котёл
//signed char tInputK; //температура на входе в котёл

#define VENPIN   5   // вентилятор на gpio05
#define PIN18B20   4   // 18b20 пин

OneWire oneWire(PIN18B20); // вход датчиков 18b20
DallasTemperature ds(&oneWire);//как ds
OneWire ds1(PIN18B20);//как ds1 для поиска датчиков

//DeviceAddress sensor_tInputK = {0x28, 0xF8, 0x63, 0x79, 0x97, 0x7, 0x3, 0x77};//40#248#99#121#151#7#3#119
DeviceAddress sensor_tInputK = {0x28, 0xFF, 0xC5, 0xA4, 0x61, 0x18, 0x2, 0x37};//40#255#197#164#97#24#2#55+


//DeviceAddress sensor_tOutputK = {0x28, 0xEF, 0x19, 0x43, 0x98, 0x18, 0x0, 0x63};//40#239#25#67#152#24#0#99
DeviceAddress sensor_tOutputK = {0x28, 0xFF, 0x12, 0x6C, 0x6C, 0x18, 0x1, 0xEF};//40#255#18#108#108#24#1#239+

//DeviceAddress sensor_tOutputC = {0x28, 0x15, 0xA9, 0x79, 0x97, 0x7, 0x3, 0x8B};//40#21#169#121#151#7#3#139
DeviceAddress sensor_tOutputC = {0x28, 0xFF, 0xE0, 0x7E, 0x6C, 0x18, 0x1, 0xF5};//40#255#224#126#108#24#1#245+

//DeviceAddress sensor_tOutputC = {0x28, 0xA0, 0x22, 0x79, 0x97, 0x7, 0x3, 0xDC};//40#160#34#121#151#7#3#220
//DeviceAddress sensor_tOutputK = {0x28, 0x15, 0xA9, 0x79, 0x97, 0x7, 0x3, 0x8B};//40#21#169#121#151#7#3#139
//
DeviceAddress sensor_tOut = {0x28, 0x31, 0x83, 0x79, 0x97, 0x6, 0x3, 0x60};//40#49#131#121#151#6#3#96+
DeviceAddress sensor_tIn = {0x28, 0xFF, 0x8B, 0xBB, 0x61, 0x18, 0x2, 0xDE};//40#255#139#187#97#24#2#222+
//DeviceAddress sensor_tInputC = {0x28, 0xEF, 0x19, 0x43, 0x98, 0x18, 0x0, 0x63};//40#239#25#67#152#24#0#99
//DeviceAddress sensor_tInputC = {0x28, 0xA0, 0x22, 0x79, 0x97, 0x7, 0x3, 0xDC};//40#160#34#121#151#7#3#220//мнимый
DeviceAddress sensor_tInputC = {0x28, 0xFF, 0x86, 0x79, 0x6C, 0x18, 0x1, 0xFE};//40#255#134#121#108#24#1#254+

DeviceAddress Thermometer;
float t[] = {0, 0, 0, 0, 0, 0};

//float dhttemp; // буферная переменная для хранения температуры от dht22
//int dhthum; // буферная переменная для хранения влажности от dht22
//int topwm; // хранит значение для шима яркости дисплея


ESP8266WebServer server(81);
const char index_html[] PROGMEM={"<!doctype html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n<title>Web Server</title>\n<meta name=\"generator\" content=\"WYSIWYG Web Builder 12 - http://www.wysiwygwebbuilder.com\">\n<link href=\"Untitled1.css\" rel=\"stylesheet\">\n<link href=\"index.css\" rel=\"stylesheet\">\n</head>\n<body>\n<div id=\"wb_Text1\" style=\"position:absolute;left:34px;top:41px;width:570px;height:23px;z-index:0;\">\n<span style=\"color:#000000;font-family:Arial;font-size:20px;\">TURN ON:</span></div>\n<div id=\"wb_Text2\" style=\"position:absolute;left:34px;top:85px;width:570px;height:23px;z-index:1;\">\n<span style=\"color:#000000;font-family:Arial;font-size:20px;\">TURN OFF:</span></div>\n<input type=\"submit\" id=\"Button1\" onclick=\"window.location.href=' ./on';return false;\" name=\"\" value=\"Submit\" style=\"position:absolute;left:146px;top:40px;width:96px;height:25px;z-index:2;\">\n<input type=\"submit\" id=\"Button2\" onclick=\"window.location.href='./off';return false;\" name=\"\" value=\"Submit\" style=\"position:absolute;left:146px;top:84px;width:96px;height:25px;z-index:3;\">\n</body>\n</html>"};
String ip = "";

bool init1(){
  SPIFFS.begin();
  File f = SPIFFS.open("/rew", "r");
  if (!f){
    str_debug +="format init/";
    SPIFFS.format();
    delay(1000);
  };
  f.close();
}

bool begin_file_config(String tmp_b1, String tmp_b2){
  str_debug +="begin_file_config_proc";
 File f=SPIFFS.open(tmp_b1, "r");
  if (f) {
      f.close();
    } else{
      File f2=SPIFFS.open(tmp_b1, "w+");
      //f2.write(tmp_b2);
      f2.println(tmp_b2);
      str_debug +=tmp_b1+" write init\n";
      f2.close();
    }
}

bool begin_config_init(){
  begin_file_config("/tboffk","81");
  begin_file_config("/tboffin","17");
  begin_file_config("/tdkc","19");
  begin_file_config("/tdkce","36");
  begin_file_config("/tdi","5");
  begin_file_config("/toc","20");
  begin_file_config("/schetPer","11");
  begin_file_config("/neg","1");
  //
  begin_file_config("/rew","0");
  //begin_file_config("tBreakOffK","");
  //begin_file_config("tBreakOffK","");
}

bool read_config(){
      File f = SPIFFS.open("/rew", "r");
    if (!f) {
        Serial.println("file open failed");
        begin_file_config("/rew","4");
        str_debug +="file rew create in read config \n";
                   //  "Открыть файл не удалось"
        
    } else{
    if (f) {
      String tmp93=f.readStringUntil('\n');
      signed char tmp103 = std::atoi (tmp93.c_str());
      str_debug += "is reader in read config ="+String(tmp103)+"\n";
      rew=tmp103;
      
    }
    f.close();
    };
}

bool read_config_main(String file_name){
    File f = SPIFFS.open(file_name, "r");
    if (!f) {
        Serial.println("file open failed");
        //begin_file_config("/rew","4");
        if (file_name =="/tboffk"){
        begin_file_config("/tboffk","79");
      };
      if (file_name =="/neg"){
        begin_file_config("/neg","1");
      };
      if (file_name =="/rew"){
        begin_file_config("rew","0");
      };
      
      if (file_name =="/tboffin"){
        begin_file_config("/tboffin","17");
      };
      if (file_name =="/tdkc"){
        begin_file_config("/tdkc","19");
      };
      if (file_name =="/tdkce"){
        begin_file_config("/tdkce","36");
      };
      if (file_name =="/tdi"){
        begin_file_config("/tdi","4");
      };
      if (file_name =="/toc"){
        begin_file_config("/toc","20");
      };
        //begin_config_init();
        
        //
        str_debug +="file  in read config not found \n";
                   //  "Открыть файл не удалось"
        
    } else{
    if (f) {
      File f2=SPIFFS.open(file_name, "r");
      String tmp_93=f2.readStringUntil('\n');
      signed char tmp_103= std::atoi (tmp_93.c_str());
      //
      str_debug += "is reader in read config ="+tmp_93+"\n";
      if (file_name =="/tboffk"){
        tBreakOffK=tmp_103;
      };
      if (file_name =="/rew"){
        rew=tmp_103;
      };
      if (file_name =="/neg"){
        if (tmp_103==1){
            negative=true;
          } else{
            negative=false;
          };
      };
      
      if (file_name =="/tboffin"){
        tBreakOffIn=tmp_103;
      };
      if (file_name =="/tdkc"){
        tDeltaKCritical=tmp_103;
      };
      if (file_name =="/tdkce"){
        tDeltaKCritical_economy=tmp_103;
      };
      if (file_name =="/tdi"){
        tDeltaIn=tmp_103;
      };
      if (file_name =="/toc"){
        tOutCritical=tmp_103;
      };
     f2.close();
    };
    f.close();
    };
}

bool read_config_real(){
  read_config_main("/rew");
  read_config_main("/neg");
  read_config_main("/tboffk");
  read_config_main("/tboffin");
  read_config_main("/tdkc");
  read_config_main("/tdkce");
  read_config_main("/tdi");
  read_config_main("/toc");
}

bool write_config_real(String file_name, String tmp){
  File f2=SPIFFS.open(file_name, "w+");
      //f2.write(tmp_b2);
      signed int tmp_222 = std::atoi (tmp.c_str());
      if (f2){
      f2.println(tmp_222);
      //
      //str_debug +="write to conffile \n";
      f2.close();
      return true;
      };
      if (!f2) return false;
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleRootPath() {
 
   server.send(200, "text/plain", "Hello world");
 
}

void wifimanstart() { // Волшебная процедура начального подключения к Wifi.
                      // Если не знает к чему подцепить - создает точку доступа ESP8266 и настроечную таблицу http://192.168.4.1
                      // Подробнее: https://github.com/tzapu/WiFiManager
  WiFiManager wifiManager;
  wifiManager.setDebugOutput(debug);
  wifiManager.setMinimumSignalQuality();
  wifiManager.setConfigPortalTimeout(333);
  wifiManager.setConnectTimeout(240);
  if (!wifiManager.autoConnect("ESP8266")) {
  if (debug) Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    //ESP.reset();
    delay(5000); }
if (debug) Serial.println("connected...");
}



 
void setup() 
{

pinMode(VENPIN,OUTPUT);
/////pinMode(14, OUTPUT); // gpio14 будет шимить вентелятор
  Hostname = "ESP"+WiFi.macAddress();
  Hostname.replace(":","");
  Serial.begin(115200);
  analogWriteFreq(3333);
  init1();
   //begin_config_init();//установка конфига по умолчанию
   //read_config();
   read_config_real();
  ds.begin();//инициализация 18b20
  ds.setResolution(sensor_tInputK,10);//18b20 установка разрешения преобразования
  ds.setResolution(sensor_tOutputK,10);//18b20 установка разрешения преобразования
  ds.setResolution(sensor_tOutputC,10);//18b20 установка разрешения преобразования
  ds.setResolution(sensor_tOut,10);//18b20 установка разрешения преобразования
  ds.setResolution(sensor_tIn,10);//18b20 установка разрешения преобразования
  ds.setResolution(sensor_tInputC,10);//18b20 установка разрешения преобразования
  Datchik();
  RegTrivial();
  global=3;
   

//byte tmo11=ds.getDeviceCount();

server.on("/specific", handlespecific);

server.on("/", [](){
    /*
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
      */
      server.send_P(200, "text/html", index_html);
      String addy = server.client().remoteIP().toString();
      if(ip != addy){
      Serial.print(addy);
      Serial.println(" is on the web");
      ip = addy;
      server.send(200, "text/html", addy);
      }
  });

  server.on("/datchik", [](){
    /*
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
      */
      //getDeviceAddress();
      //_18b20search();
      //printAddress;
      String message="<head></head><body>";
      message+="<table>";
      for( i = 0; i < 6; i++) {
        ds.getAddress(Thermometer, i);
        t[i]= ds.getTempCByIndex(i);
        delay(200);
        message+=tr_td;
        message+="id=";
        message+=i;
        message+="|"+_td_td;
        //message+="<tr><td>";
        for (uint8_t ii = 0; ii < 8; ii++)
        {
          // zero pad the address if necessary
          message+=Thermometer[ii];
          message+="#";
        }
        //message+=Thermometer;
        //message+="nnn";
        //message+=addr[i];
        //message+="mmm";
        //message+=t[i];
        message+="|"+_td_td+"|"+_td_td;
        //const uint8_t tmp4=addr[i];
        //DeviceAddress tmp4={addr[i]};
        //message+=ds.getTempC(tmp4);
        message+=t[i];
        //message+="</td></tr>";
        message+=_td__tr;
        //message+="</br>";
  };
      message+="</table>";
      message+="</body></html>";
      server.send(200, "text/html", message);
  });




server.on("/otdacha", [](){
    /*
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
      */
      signed char schet2;
      schet2=schet;
      if (schet==0){
        schet2=1;
      }
      String message;
      message+=String(tInputK/schet2);
      message+="\n";
      message+=String(tOutputK/schet2);
      message+="\n";
      message+=String(tInputC/schet2);
      message+="\n";
      message+=String(tOutputC/schet2);
      message+="\n";
      message+=String(tIn/schet2);
      message+="\n";
      message+=String(tOut/schet2);
      message+="\n";
      message+=String(schet2);
      message+="\n";
      message+=String(kof);
      message+="\n";
      message+=String(tmp1);
      message+="\n";
      message+=String(rew);
      message+="\n";
      message+=String(razgon_tmp);
      message+="\n";
      message+=String(razgon);
      message+="\n";
      message+=String(ryj);
      message+="\n";
      message+=String(tBreakOffK);
      message+="\n";
      message+=String(tBreakOffIn);
      message+="\n";
      message+=String(tDeltaKCritical);
      message+="\n";
      message+=String(tDeltaIn);
      message+="\n";
      message+=String(tOutCritical);
      message+="\n";
      message+=String(tDeltaKCritical_economy);
      message+="\n";
      message+=String(razgon);
      message+="\n";
      message+=String(negative);
      message+="\n";
      message+=String(zalp_per);
      message+="\n";
      server.send(200, "text/plain", message);
  });
  

  
  server.on("/status", [](){
    /*
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    digitalWrite(led, LOW);
    */
    signed char schet2;
      schet2=schet;
      if (schet==0){
        schet2=1;
      }
    String message="<!doctype html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n<meta http-equiv=\"refresh\" content=\"3;/status\"> \n</head>\n";
    message+="<body>\n<div style=\"position:absolute;left:20px;top:19px;width:250px;height:24px;z-index:0;\">\n<span style=\"color:#000000;font-family:Arial;font-size:21px;\">";
    message+="tInputK=";//tInputK
    message+=String(tInputK/schet2);
    message+="</br>";
    message+="tInputC=";//tInputC
    message+="";
    message+=String(tInputC/schet2);
    message+="</br>";
    message+="tOutputK=";//tOutputK
    message+="";
    message+=String(tOutputK/schet2);
    message+="</br>";
    message+="tOutputC=";//tOutputC
    message+="";
    message+=String(tOutputC/schet2);
    message+="</br>";
    message+="tOut=";//tOut
    message+="";
    message+=String(tOut/schet2);
    message+="</br>";
    message+="tIn=";
    message+="";
    message+=String(tIn/schet2);
    message+="</br>";
    message+="kof=";
    message+=String(kof);
    message+="</br>";
    message+="oborot=";
    message+=String(tmp1);
    message+="</br>";
    //message+=String(tInputK/schet);
    message+="</span></div>\n</body>\n</html>";
    server.send(200, "text/html", message);
    Serial.println("The LED is now ON");
  });
  // инициализация экрана
  //lcd.begin(4, 5);  // sda=gpio04, scl=gpio05
  //lcd.backlight();

  
  //ds.getDeviceCount
  //ds.getAddress
  // инициализация датчика температуры и давления bmp180
  /*
  dht.begin();
  if (!bmp.begin()) // если датчик не обнаружен, сообщаем об этом в компорт и на дисплей
  {
    Serial.println("Could not find BMP180 or BMP085 sensor at 0x77");
    //lcd.clear();
    //lcd.print("BMP180 FAILED");
    while (1) {}
  }
  */

  server.begin();
  WiFi.hostname(Hostname);
  wifimanstart();
  Serial.println(WiFi.localIP()); Serial.println(WiFi.macAddress()); Serial.print("Narodmon ID: "); Serial.println(Hostname); // выдаем в компорт мак и айпишник железки, так же выводим на дисплей
  /*
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(WiFi.localIP());
  lcd.setCursor(0, 1);
  lcd.print(Hostname);
  */
  lastConnectionTime = millis() - postingInterval + 15000; //первая передача на народный мониторинг через 15 сек.
}

 /*
 void WriteLcdTemp (void){ // заполнение дисплея. происходит каждые 5 минут после получения данных с датчиков
 lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("T1 ");
  lcd.setCursor(4, 0);
  lcd.print(bmp.readTemperature());

lcd.setCursor(8,0);
lcd.print(" P ");
lcd.setCursor(11,0);
lcd.print(bmp.readPressure()/133.3);

  lcd.setCursor(1, 1);
  lcd.print("T2 ");
  lcd.setCursor(4, 1);
  lcd.print(dhttemp);

  lcd.setCursor(8,1);
lcd.print(" H ");
lcd.setCursor(11,1);
lcd.print(dhthum);
lcd.setCursor(13,1);
lcd.print("%");
  }
*/
bool RegTrivial() { //регулировка оборотов вентелятора

  //отсчёт счёта
  //schet=schet+1;

if (zalp_c>100){
  zalp_c=1;
}
  
Serial.println(schet);
if (schet==schetPer){


  
zalp_c=zalp_c+1;
  
Serial.println(schet);
delay(200);
  //делаем медиану
  tInputK=tInputK/schetPer;
  tOutputK=tOutputK/schetPer;
  tOutputC=tOutputC/schetPer;
  tOut=tOut/schetPer;
  tIn=tIn/schetPer;
  tInputC=tInputC/schetPer;
  //переменную schet в 1 ибо нынешние показания примутся как первые в отсчёте
  schet=1;
  Serial.println(schet);

//float kof=293/(tOut+273)*(1.1+(293-(tOut+273))/50);//температурный поправоный коэффициент роста оборотов

kof=-5.86*(tOut-21.1)/(tOut+273);//температурный поправоный коэффициент роста оборотов
// -(x-30)*(x-30)*(x-30)/70000

signed short tmp2=((short) tBreakOffIn)-tIn;//для удобства текущая дельта между допустимой внутренней и реальной
signed short tmp3 = 512;//переменная для оптимального
signed short tmp4 = 512;//переменная для оптимального


if (tOutputK<(tBreakOffK-tDeltaKCritical)){
  tmp3=(short)(((1023/((float)tDeltaIn))*((float)tmp2)*kof)/2);//в 2 раза медленнее чем ниже приведённая
  if (tmp3<400){ //не менее 500(половина скорости вентелятора)
    tmp3=412;
  };
};

if (tmp2>tDeltaIn){//есль больше заданной дельты, то заданная дельта
  tmp2= (short) tDeltaIn;
  Serial.println("ne bolee deljta");
};
if (tmp2<0){//если меньше нуля, то 0
  tmp2=0;
  Serial.println("he menee 0");
};
if (((short)tBreakOffIn) > tIn and rew == 1){//если температура внутри менее допустимой и режим первый
  tmp1=(short) (((1023/tDeltaIn)*tmp2*kof)/1);//максимальный диапазон икм делим на дельту заданную, затем * на tmp2(не более дельты) и умножаем на коэффициент зависищий от забортной температуры для нелинейности регулировки
};

if (tmp3 > tmp1 and tOutputK<(tBreakOffK-tDeltaKCritical)){//если алгоритм для оптимальности котла более и выход из котла менее оптимальной
  tmp1=tmp3;
  Serial.println("optimum in prioritet");
};

if (rew==0 and tmp1<tmp3){//если не первый режим
  tmp1=tmp3;
  Serial.println("balans");
}

if (rew==3){//если 3 режим режим таблицы

  if (tIn>10 and tIn<12){
    tmp4=1023;
  }
  else if (tIn>11 and tIn<13){
    tmp4=1020;
  }
  else if (tIn>12 and tIn<14){
    tmp4=1000;
  }
  else if (tIn>13 and tIn<16){
    tmp4=900;
  }
  else if (tIn>15 and tIn<17){
    tmp4=800;
  }
  else if (tIn>17 and tIn<19){
    tmp4=500;
  }
  else if (tIn>18 and tIn<20){
    tmp4=400;
  }
  else if (tIn>19 and tIn<21){
    tmp4=300;
  }
  else if (tIn>20 and tIn<22){
    tmp4=23;
  }
  else if (tIn>21){
    tmp4=0;
  } else {
    tmp4=0;
  }
  
  tmp1=tmp4;
  Serial.println("table");
}

if (rew==4){//если 4 режим полной экономии
  signed short tmp5;
  if (tIn>9 and tOutputK>(tBreakOffK-tDeltaKCritical_economy)){
    tmp5=0;
  } else{
    tmp5=1022;
  };
  if (tOutputK>tBreakOffK){
    tmp5=0;
  };
  tmp1=tmp5;
}

if (rew==5){//по отсечке температуры внутри помещения и с учётом минимальной температурой котла
  signed short tmp6;
  if (tIn>tBreakOffIn-2 and tIn<tBreakOffIn and tOutputK>(tBreakOffK-tDeltaKCritical_economy)){
    tmp6=512;
  } else if (tIn>=tBreakOffIn and tOutputK>(tBreakOffK-tDeltaKCritical_economy)){
    tmp6=0;
  } else{
    tmp6=1023;
  };
  if (tOutputK>tBreakOffK){
    tmp6=0;
  };
  tmp1=tmp6;
}

if (rew==6){//по отсечке температуры внутри помещения без учёта минимальной температурой котла+
  signed short tmp7;
  if (tIn>tBreakOffIn-2 and tIn<tBreakOffIn ){
    tmp7=512;
  } else if(tIn<(tBreakOffIn-2)){
    tmp7=1023;
  } else{
    tmp7=0;
  };
  if (tOutputK>tBreakOffK){
    tmp7=0;
  };
  tmp1=tmp7;
}

if (rew==7){//по отсечке температуры котла+
  signed short tmp8;
  if (tOutputK>(tBreakOffK-4) and tOutputK<=(tBreakOffK-2)) {
    tmp8=733;
  } else if (tOutputK>(tBreakOffK-2) and tOutputK<=tBreakOffK){
    tmp8=400;
  } else if (tOutputK>tBreakOffK) {
    tmp8=0;
  } else{
    tmp8=1023;
  };
  if (tOutputK>tBreakOffK){
    tmp8=0;
  };
  tmp1=tmp8;
}

if (rew==21){//21 режим
  tmp1=ryj;
  Serial.println("ryj");
}
//залп
 if (rew>3){
   if (rew<21){
   if (zalp){
    if (zalp_per){
      if (tOutputK<71){
        Serial.println("zalp OK");
       tmp1=1023; 
      }
    }
   }
   }
 }
 
  if (zalp_per){
   zalp_per=false;
 } else{
    if (zalp_c == 4){
      Serial.println("zalp_c=4");
      zalp_per=true;
      zalp_c=1;
    }
 }
 
//операция оптимизации и проверки
if (tmp1 > 1023){//чтоб было не более 1023(от 0 до 1023)
  tmp1=1023;
  Serial.println("max oborot");
}

if (tOut>tOutCritical){//если температура на улице выше допустимой
  tmp1=0;
  Serial.println("tOut>tOutCritical->off ven");
};

if (tIn>tBreakOffIn and tOutputK > (tBreakOffK-tDeltaKCritical)){//если температура внутри больше допустимой И температура на выходе из котла более минимальной оптимальной
  tmp1=0;
  Serial.println("tkotla optimaljna i t vnutri optimaljna->off ven");
};
//конец операция оптимизации и проверки

//операция разгон котла
if (tOutputK<33 and tOut<15){
  razgon_tmp=true;
};
if (tOutputK>47){
  razgon_tmp=false;
};
if (razgon and razgon_tmp) {
  tmp1=1023;
};
//конец операция разгон котла

//операция защиты от перегрева

if (tOutputK>tBreakOffK){//если температура на выходе из котла более допустимой для котла
  tmp1=0;
  Serial.println("tOutputK>tBreakOffK");
  };

if (tOutputK>95){//защита от дурака чтоб котёл не бабахнул
  tmp1=0;
  Serial.println("warning!!! temperatyra koTLa bolee 95. ventiljtor to off!!!");
  };

//конец операция защиты от перегрева


if (negative){//если драйвер вентелятора управляется негативными импульсами
  tmp1=1024-tmp1;//
}
//tmp1=tmp1/4;
analogWrite(VENPIN,tmp1);
Serial.println(tmp1);
};
return true; //ушло
};
bool Datchik(){//опрос датчиков
  
  //ds.requestTemperatures(); // считываем температуру с датчиков
  //delay(1000);//необходимо не менее 750
  //#define ETS_INTR_LOCK() ets_intr_lock() //запрет прерываний
  signed short  tmp_tInputK=0;
  signed short tmp_tOutputK=100;
  signed short tmp_tOutputC=0;
  signed short tmp_tOut=20;
  signed short tmp_tIn=16;
  signed short tmp_tInputC=0;
  
  
  if (indexforindex==0){
  if (ds.isConnected(sensor_tInputK)){
    //#define ETS_INTR_LOCK() ets_intr_lock() //запрет прерываний
  ds.requestTemperaturesByAddress(sensor_tInputK);
  //delay(200);
  tmp_tInputK= (short) ds.getTempC(sensor_tInputK);
  //#define ETS_INTR_UNLOCK() ets_intr_unlock() //разрешение всех прерываний
  }

  if (ds.isConnected(sensor_tOutputC)){
    //#define ETS_INTR_LOCK() ets_intr_lock() //запрет прерываний
  ds.requestTemperaturesByAddress(sensor_tOutputC);
  //
  delay(200);
  tmp_tOutputC= (short) ds.getTempC(sensor_tOutputC);
  //#define ETS_INTR_UNLOCK() ets_intr_unlock() //разрешение всех прерываний
  }

  
  if (ds.isConnected(sensor_tOutputK)){
    //#define ETS_INTR_LOCK() ets_intr_lock() //запрет прерываний
  ds.requestTemperaturesByAddress(sensor_tOutputK);
  //
  delay(200);
  tmp_tOutputK= (short) ds.getTempC(sensor_tOutputK);
  //#define ETS_INTR_UNLOCK() ets_intr_unlock() //разрешение всех прерываний
  }

  if (ds.isConnected(sensor_tOut)){
    //#define ETS_INTR_LOCK() ets_intr_lock() //запрет прерываний
  ds.requestTemperaturesByAddress(sensor_tOut);
  //
  delay(200);
  tmp_tOut= (short) ds.getTempC(sensor_tOut);
  //#define ETS_INTR_UNLOCK() ets_intr_unlock() //разрешение всех прерываний
  }
  
  if (ds.isConnected(sensor_tIn)){
    //#define ETS_INTR_LOCK() ets_intr_lock() //запрет прерываний
  ds.requestTemperaturesByAddress(sensor_tIn);
  //#define ETS_INTR_UNLOCK() ets_intr_unlock() //разрешение всех прерываний
  //delay(200);
  tmp_tIn= (short) ds.getTempC(sensor_tIn);
  //#define ETS_INTR_UNLOCK() ets_intr_unlock() //разрешение всех прерываний
  }
  
  if (ds.isConnected(sensor_tInputC)){
    //#define ETS_INTR_LOCK() ets_intr_lock() //запрет прерываний
  ds.requestTemperaturesByAddress(sensor_tInputC);
  //
  delay(200);
  tmp_tInputC= (short) ds.getTempC(sensor_tInputC);
  //#define ETS_INTR_UNLOCK() ets_intr_unlock() //разрешение всех прерываний
  }
  
    //защита от обрыва датчика
if (tmp_tOutputK<-54){
  tmp_tOutputK=100;
};
    
  };//

  
  //Serial.println("Not connected to WiFi"); 
  Serial.println(tInputK);
  Serial.println(tOutputK);
  Serial.println(tOutputC);
  Serial.println(tOut);
  Serial.println(tIn);
  Serial.println(tInputC);


  
//накапливаем в переменную
  tInputK=tInputK+tmp_tInputK;
  tOutputK=tOutputK+tmp_tOutputK;
  tOutputC=tOutputC+tmp_tOutputC;
  tOut=tOut+tmp_tOut;
  tIn=tIn+tmp_tIn;
  tInputC=tInputC+tmp_tInputC;
  Serial.println(schet);

  
  //PIN18B20
  //#define ETS_INTR_UNLOCK() ets_intr_unlock() //разрешение всех прерываний
  schet=schet+1;//количество опросов
  return true; //ушло
}
bool SendToNarodmon() { // Собственно формирование пакета и отправка.
    WiFiClient client;
    String buf;
    buf = "#" + Hostname + "#ESP_YOBA" + "\r\n"; // заголовок И ИМЯ, которое будет отображаться в народном мониторинге, чтоб не палить мак адрес
//dhttemp=dht.readTemperature(); // сохравняем в буферные переменные данные с dht22, чтобы удобно было оперировать
//dhttemp=14;
//dhthum=dht.readHumidity();
//dhthum=17;
    //buf = buf + "#T1#" + String(bmp.readTemperature()) + "\r\n"; //температура bmp180
    buf = buf + "#tInputK#" + String(tInputK/schet) + "\r\n"; //температура bmp180
    buf = buf + "#tOutputK#" + String(tOutputK/schet) + "\r\n"; //температура dht22
    
    buf = buf + "#tOutputC#" + String(tOutputC/schet) + "\r\n"; //tOutputC
    buf = buf + "#tOut#" + String(tOut/schet) + "\r\n"; //tOut
    buf = buf + "#tIn#" + String(tIn/schet) + "\r\n"; //tIn
    buf = buf + "#tIn#" + String(tInputC/schet) + "\r\n";//tInputC
    buf = buf + "##\r\n"; // закрываем пакет
 
    if (!client.connect("narodmon.ru", 8283)) { // попытка подключения
      Serial.println("connection failed");
      //lcd.clear();
      //lcd.print("connect failed");
      return false; // не удалось;
    } else
    {
      //WriteLcdTemp();
      client.print(buf); // и отправляем данные
      if (debug) Serial.print(buf);
      while (client.available()) {
        String line = client.readStringUntil('\r'); // если что-то в ответ будет - все в Serial
        Serial.print(line);
        
        }
    }
      return true; //ушло
      
  }
bool webserver(){
  return true; //ушло
}



void handlespecific() { //изменение
  String message="";
  if (server.arg("tBreakOffK_")==""){
    message +="tBreakOffK";
  };
  if (server.hasArg("tBreakOffK_")) {
    message="tBreakOffK Argument=";
    String tmp9=server.arg("tBreakOffK_");
    signed char tmp10 = std::atoi (tmp9.c_str());//only signed char иначе не пишет в конфиг
    if (tmp10>94 or tmp10<50){
      tmp10=80;
    };
    String tmp_str=String(tmp10);
    write_config_real("/tboffk",tmp_str);
    message +=server.arg("tBreakOffK_");
    tBreakOffK=tmp10;//внимание. приравнивание апосля всего иначе замена значений в силу одинаковых имён переменных.
    
  };

//tBreakOffIn = 19
  if (server.arg("tBreakOffIn_")==""){
    message +="tBreakOffIn";
   };
  if (server.hasArg("tBreakOffIn_")) {
    message="tBreakOffIn Argument=";
    String tmp90=server.arg("tBreakOffIn_");
    signed char tmp100 = std::atoi (tmp90.c_str());
    if (tmp100>37 or tmp100<5){
      tmp100=16;
    };
    String tmp_str=String(tmp100);
    write_config_real("/tboffin",tmp_str);
    message +=server.arg("tBreakOffIn_");
    tBreakOffIn=tmp100;//error global
    
  };

//tDeltaKCritical = 18

  if (server.arg("tDeltaKCritical_")==""){
    message +="tDeltaKCritical";
    };
  if (server.hasArg("tDeltaKCritical_")) {
    message="tDeltaKCritical=";
    String tmp900=server.arg("tDeltaKCritical_");
    signed char tmp1000 = std::atoi (tmp900.c_str());
    if (tmp1000>40 or tmp1000<5){
      tmp1000=18;
    };
    String tmp_str=String(tmp1000);
    write_config_real("/tdkc",tmp_str);
    //tmp10 = server.arg("tBreakOffK");
    message +=server.arg("tDeltaKCritical_");
    tDeltaKCritical=tmp1000;
    
  };

//tDeltaIn=4
  if (server.arg("tDeltaIn_")==""){
    message +="tDeltaIn";
    };
  if (server.hasArg("tDeltaIn_")) {
    message="tDeltaIn Argument=";
    String tmp91=server.arg("tDeltaIn_");
    signed char tmp101 = std::atoi (tmp91.c_str());
    if (tmp101>20 or tmp101<2){
      tmp101=4;
    };
    String tmp_str=String(tmp101);
    write_config_real("/tdi",tmp_str);
    //tmp10 = server.arg("tBreakOffK");
    message +=server.arg("tDeltaIn_");
    tDeltaIn=tmp101;
    
  };

//tOutCritical=27

  if (server.arg("tOutCritical_")==""){
    message +="tOutCritical";
    };
  if (server.hasArg("tOutCritical_")) {
    message="tOutCritical Argument=";
    String tmp92=server.arg("tOutCritical_");
    signed char tmp102 = std::atoi (tmp92.c_str());
    if (tmp102>30 or tmp102<5){
      tmp102=27;
    };
    String tmp_str=String(tmp102);
    write_config_real("/toc",tmp_str);
    //tmp10 = server.arg("tBreakOffK");
    message +=server.arg("tOutCritical_");
    tOutCritical=tmp102;//error global to be
    
  };

  if (server.arg("pew_")==""){
    message +="pew fault";
    };
  if (server.hasArg("pew_")) {
    message="pew Argument=";
    String tmp93=server.arg("pew_");
    //
    signed char tmp103 = std::atoi (tmp93.c_str());
   
      //
    switch (tmp103){
      case 0:
        rew=0;
        break;
      case 1:
        rew=1;
        break;
      case 2:
        rew=2;
        break;
      case 3:
        rew=3;
        break;
      case 4:
        rew=4;
        break;
      case 5:
        rew=5;
        break;
      case 6:
        rew=6;
        break;
      case 7:
        rew=7;
        break;
       case 21:
        rew=21;
        break;
      default:
        rew=1;
        break;
    };
    String tmp_str=String(tmp103);
    write_config_real("/rew",tmp_str);
    message +=server.arg("pew_");
      message +="ok"+str_debug;
    //rew=tmp103; 
  };

  if (server.arg("pyj_")==""){
    message +="pyj fault";
  };
  if (server.hasArg("pyj_")) {
    message="pyj Argument=";
    String tmp94=server.arg("pyj_");
    signed short tmp104 = std::atoi (tmp94.c_str());
    if (tmp104>1023 or tmp104<0){
      tmp104=10;
    };
    ryj=tmp104;
    String tmp_str=String(tmp104);
    message +=server.arg("ryj_");
    
  };
  //
  if (server.arg("format")==""){
    message +="format";
    };
  if (server.hasArg("format")) {
    //bool tmp175=true;
    message="negative Argument=";
    String tmp1194=server.arg("format");
    char tmp1174 = std::atoi (tmp1194.c_str());
    if (tmp1174>0){
      //bool tmp175=true;
      SPIFFS.format();
      message +=server.arg("format");
      delay(1000);
      ESP.restart();
      
    };
    //negative=tmp175;
    //tmp10 = server.arg("tBreakOffK");
    message +=server.arg("format");
    
  };

  if (server.arg("negative_")==""){
    message +="negative fault";
    };
  if (server.hasArg("negative_")) {
    //bool tmp175=true;
    message="negative Argument=";
    String tmp94=server.arg("negative_");
    signed char tmp174 = std::atoi (tmp94.c_str());
    if (tmp174>0){
      //bool tmp175=true;
      negative=true;
      write_config_real("/neg","1");
    }
    else {
      //bool tmp175=false;
      negative=false;
      write_config_real("/neg","0");
    };
    message +=server.arg("negative_");
    
  };


server.send(200, "text/plain", message);
}


void loop()

{
  if (millis() > 700000000){
    ESP.restart();
  }
//global +=1;
if (global==1){
   //global=3;
   //init1();
   //begin_config_init();
   //read_config();
   //read_config_real;
};
Serial.println("ver:0.10");
if (millis() - lastTime > 17000){//раз в 17 секунду опрос и счёт
  
Datchik();
RegTrivial();
lastTime = millis();
};
//
server.handleClient();
//автояркость
//topwm=map(analogRead(A0), 0, 1023, 10, 950); 
//analogWrite(14, topwm);
  delay(100);// нужна, беж делея у меня не подключался к вайфаю

 
  if (millis() - lastConnectionTime > postingInterval) {// ждем 5 минут и отправляем

      
      if (WiFi.status() == WL_CONNECTED) { // ну конечно если подключены
      if (SendToNarodmon()) {
      lastConnectionTime = millis();
      }
      else{  lastConnectionTime = millis() - postingInterval + 15000;
      }//следующая попытка через 15 сек    
      }
      else{
      lastConnectionTime = millis() - postingInterval + 15000; 
      Serial.println("Not connected to WiFi"); 
      //lcd.clear(); 
      //lcd.print("No WiFi");
      }//следующая попытка через 15 сек
    } yield(); // что за команда - фиг знает, но ESP работает с ней стабильно и не глючит.
}
