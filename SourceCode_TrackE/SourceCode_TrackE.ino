#include <Arduino.h>
#include <Stream.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

#define RXnb_pin  16
#define TXnb_pin  17

#define RXlora_pin  18
#define TXlora_pin  19

#define RXgps_pin  13
#define TXgps_pin  14

// The serial connection to the nb-iot
Stream *_Serial;
HardwareSerial myserial(1);

// The TinyGPS++ object
TinyGPSPlus gps;
// The serial connection to the GPS device
HardwareSerial ss(2);

//var GPS
struct location{
  float lat;
  float lng;
  String datetime;
  int reason;
};
location data;

//nb-iot
struct _RES{
  unsigned char status;
  String data;
  String temp;
};
int cmm_state = 0;
String cmm_reason = "";
int count = 0;

int time_sleep;

//device_id
uint64_t chipid;
String real_chip;

unsigned long last_time;
bool ForcePass = false;

//****************************Lora****************************
bool send_sucress = false;
bool start_send = false;
bool repeat_enable = false;

unsigned long previousMillis = 0;
unsigned long currentMillis = millis();
const long interval = 30000;

unsigned long previousMillis2 = 0;

const long interval2 = 60000;

String str;
//************************************************************


void setup() {
  Serial.begin(115200);
  ss.begin(9600, SERIAL_8N1, RXgps_pin, TXgps_pin, false);
  chipid = ESP.getEfuseMac();
  last_time = millis() + 10000;
  pinMode(26, OUTPUT);  //nb-iot pwrkey
  pinMode(27, OUTPUT);  //nb-iot reset

  Serial.println("Wait GPS Signal...");
}

void loop() {
  while (ss.available() > 0)
  {
    if (gps.encode(ss.read()))
    {
      if (gps.location.isValid() or ForcePass) 
      {
        //*******************read GPS data*******************
        data.reason = 2;
        data.lat = gps.location.lat();
        data.lng = gps.location.lng();
        if((data.lat >= 14.15468 && data.lat <= 14.15528)&&(data.lng >= 101.36459 && data.lng <= 101.36560)){
          time_sleep = 66;
        } else {
          time_sleep = 65;
        }
        data.datetime = String(gps.date.year()) + "-" + String(gps.date.month()) + "-" + String(gps.date.day()) + " " + String(gps.time.hour() + 7) + ":" + String(gps.time.minute())+ ":" + String(gps.time.second());
        ss.end();
        //****************************************************

        //******************* init device id*******************
        char cp[12];
        sprintf(cp,"%04X%08X",(uint16_t)(chipid>>32),(uint32_t)chipid);
        real_chip = String(cp);
        Serial.print("device id : ");
        Serial.println(real_chip);
        //****************************************************

        //power on module nb
        digitalWrite(26, LOW);
        delay(800);
        digitalWrite(26, HIGH);
        delay(800);

        //begin nb-iot module
        myserial.begin(115200, SERIAL_8N1, RXnb_pin, TXnb_pin,false);
        _Serial = &myserial;

        //******************************nb-iot send****************************
        Serial.println("START.......");
        /*Send_command("AT");
        Serial.println("Command Status : " + String(cmm_state));
        Send_command("AT+CIMI");
        Serial.println("Command Status : " + String(cmm_state));*/
        Send_command("AT+CPIN?");
        Serial.println("Command Status : " + String(cmm_state));
        Send_command("AT+CFUN=1");
        Serial.println("Command Status : " + String(cmm_state));
        Send_command("AT*MCGDEFCONT=\"IPV4V6\",\"\"");
        Serial.println("Command Status : " + String(cmm_state));

        int ernb_flag = 0;
        //check nbito error
        if(cmm_state != 1)
        {
          ernb_flag++;
        }
        
        //Send_command("AT+CSQ");
        //Serial.println("CSQ Command Status : " + String(cmm_reason));
        Send_command("AT+CGREG=1");
        Serial.println("Command Status : " + String(cmm_state));
        Send_command("AT+CGATT=1");
        Serial.println("Command Status : " + String(cmm_state));
        Send_command("AT+CGCONTRDP");
        Serial.println("Command Status : " + String(cmm_state));

        //check nbito error
        if(cmm_state != 1)
        {
          ernb_flag++;
        }
        
        //Send_command("AT+CHTTPCREATE=\"http://34.87.29.74/api/\"");
        Send_command("AT+CHTTPCREATE=\"http://150.95.27.47/plesk-site-preview/smartkhaoyai.com/https/150.95.27.47/api\"");
        Serial.println("Command Status : " + String(cmm_state));

        //check nbito error
        if(cmm_state != 1)
        {
          ernb_flag++;
        }
        
        Send_command("AT+CHTTPCON=0");
        Serial.println("1Command Status : " + String(cmm_state));

        //check nbito error
        if(cmm_state != 1)
        {
          ernb_flag++;
        }
        
        //Serial.println(ernb_flag);
        // check nbiot swict LoRa
        if(ernb_flag > 0)
        {
          //reset nbiot module
          digitalWrite(27, LOW);
          delay(800);
          digitalWrite(27, HIGH);
          
          //lora begin
          myserial.begin(115200, SERIAL_8N1, RXlora_pin, TXlora_pin);
          
          //******************************LoRa send****************************
          myserial.println("AT+NRB");
          delay(1000);
          Read_serial();
          myserial.println("AT+ADDR=12EF0236");
          Read_serial();
          delay(100);
          myserial.println("AT+APPEUI=12EF023612212313");
          Read_serial();
          delay(200);
          myserial.println("AT+APPSKEY=1628AE2B7E15D2A6ABF7CF4F3C158809");
          Read_serial();
          delay(100);
          myserial.println("AT+NWKSKEY=28AED22B7E1516A609CFABF715884F3C");
          Read_serial();
          delay(100);
          myserial.println("AT+ISMBAND=2");
          Read_serial();
          delay(100);
          myserial.println("AT+CHMASK=00FF,0000,0000,0000,0000,0000");
          Read_serial();
          delay(100);
        
          myserial.println("AT+RXWIN2");
          Read_serial();
          delay(1000);
        
          myserial.println("AT+RXWIN2=923200000,2");
          Read_serial();
          delay(100);
        
          myserial.println("AT+CLASS=C");
          Read_serial();
          delay(100);
          myserial.println("AT+ACTIVATE=0");
          Read_serial();
          delay(100);
          myserial.println("AT+ADR=0");
          Read_serial();
          delay(100);
          myserial.println("AT+DR=0");
          Read_serial();
          delay(100);
          myserial.println("AT+CFM=1");
          Read_serial();
          delay(100);
          myserial.println("AT+POWER=0");//normal 0
          Read_serial();
          delay(100);
          myserial.println("AT+CLASS");
          Read_serial();
          delay(100);
        
          myserial.println("AT+CHSET?");
          Read_serial();
          delay(100);
        
          myserial.println("AT+NCONFIG");
          Read_serial();
          delay(100);

          str = String(data.lat,6)+","+String(data.lng,6)+","+data.datetime+","+real_chip;
          start_Senddata();
          Send_data();
          Read_serial();
          DeepSleep(time_sleep);
        }
        
        static String Payload = "{\"lat\":\"" + String(data.lat,6) + "\",\"lng\":\""+String(data.lng,6)+"\",\"time_in\":\""+data.datetime+"\",\"device_id\":\""+real_chip+"\"}";
        Payload = str2HexStr(Payload);
        //Serial.println("Payload :" + Payload);
        
        //Send_command("AT+CHTTPSEND=0,1,\"/Test_nb_rec.php\",4163636570743a202a2f2a0d0a436f6e6e656374696f6e3a204b6565702d416c6976650d0a557365722d4167656e743a2053494d434f4d5f4d4f44554c450d0a,\"application/json\"," + Payload);
        Send_command("AT+CHTTPSEND=0,1,\"/collar_rec.php\",4163636570743a202a2f2a0d0a436f6e6e656374696f6e3a204b6565702d416c6976650d0a557365722d4167656e743a2053494d434f4d5f4d4f44554c450d0a,\"application/json\"," + Payload);
        Serial.println("2Command Status : " + String(cmm_state));
        
        delay(2000);    //wait nbiot send data 2 sec
        
        Send_command("AT+CHTTPDISCON=0");
        Serial.println("Command Status : " + String(cmm_state));
        Send_command("AT+CHTTPDESTROY=0");
        Serial.println("Command Status : " + String(cmm_state));

        //reset nbiot module
        digitalWrite(27, LOW);
        delay(800);
        digitalWrite(27, HIGH);

        DeepSleep(time_sleep);
        
      }else if(last_time <= millis())    //no gps signal in 10 sec
      {
        ForcePass = true;
      }
      break;
    }
  }
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    ss.end();
    DeepSleep(180);
  }

}

//***************************************nbiot***************************************
String  str2HexStr(String strin) {
  int lenuse = strin.length();
  char charBuf[lenuse * 2 - 1];
  char strBuf[lenuse * 2 - 1];
  String strout = "";
  strin.toCharArray(charBuf, lenuse * 2) ;
  for (int i = 0; i < lenuse; i++)
  {
    sprintf(strBuf, "%02X", charBuf[i]);

    if (String(strBuf) != F("00") )
    {
      strout += strBuf;
    }
  }

  return strout;
}

String Wait_module_res(long tout, String str_wait) {
  unsigned long pv_ok = millis();
  unsigned long current_ok = millis();
  String input;
  unsigned char flag_out = 1;
  unsigned char res = -1;
  _RES res_;
  res_.temp = "";
  res_.data = "";

  while (flag_out)
  {
    if (_Serial->available())
    {
      input = _Serial->readStringUntil('\n');
      res_.temp += input;
      if (input.indexOf(str_wait) != -1)
      {
        res = 1;
        cmm_state = 1;
        flag_out = 0;
      }
      else if (input.indexOf(F("ERROR")) != -1)
      {
        res = 0;
        cmm_state = 0;
        flag_out = 0;
      }
    }
    current_ok = millis();
    if (current_ok - pv_ok >= tout)
    {
      flag_out = 0;
      res = 0;
      pv_ok = current_ok;
    }
  }
  res_.status = res;
  res_.data = input;
  return (res_.temp);
}

void Send_command(String cmd) {
  String Sim_res = "";
  cmm_reason = "";
  int fail_cunt = 0;
  _Serial->println(cmd);
  do
  {
    cmm_state = -1;
    
    Sim_res = Wait_module_res(500, "OK");
    fail_cunt ++;
    if (fail_cunt > 5) break;
    Serial.print(".");
    delay(300);
  } while (cmm_state != 1);

  //Sim_res.replace("OK", "");
  cmm_reason = Sim_res;
}
//***********************************************************************************

//***************************************lora***************************************
void start_Senddata()
{
  start_send = true;
  send_sucress = false;
}

void Send_data()
{
  if (start_send == true)
  {
    start_send = false;
    myserial.println("AT+NCMGS=" + String(str.length()) + "," + str);
    repeat_enable = true;
    currentMillis = millis();
    previousMillis = currentMillis;
  }
  if (repeat_enable == true)
  {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
      previousMillis = currentMillis;
      if (send_sucress == false)
      {
        myserial.println("AT+NCMGS=" + String(str.length()) + "," + str);
      }
    }
  }
}

void Read_serial()
{
  if (myserial.available())
  {
    while (myserial.available() > 0)
    {
      String text = myserial.readStringUntil('\n');
      if (text.indexOf(F("+NNMI:3")) != -1)
      {
        send_sucress = true;
        repeat_enable = false;
        Serial.println("Send Sucress...");
      }
      if (text.indexOf(F("+ACK")) != -1)
      {
        send_sucress = true;
        repeat_enable = false;
        Serial.println("Send Sucress...");
      }
      Serial.println(text);
    }
  }
}
//**********************************************************************************
void DeepSleep(int TIME_TO_SLEEP) {
  uint64_t sleeptime = UINT64_C(TIME_TO_SLEEP * 1000000);
  Serial.print("Deep Sleep : ");
  Serial.println(TIME_TO_SLEEP);

  esp_sleep_enable_timer_wakeup(sleeptime);
  esp_deep_sleep_start();
}
