#include <stdlib.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <EEPROM.h>

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

volatile char receivedString[128];
volatile uint8_t receivedIndex = 0;
volatile bool stringComplete = false;

#define led_colagen 5
#define rele_start A5
#define rele_cap A1
#define task 3
//#define forward 4
#define pin_start 4
#define lamp 2
#define back A2
#define UP HIGH
#define DOWN LOW

#define IR_1 6
#define IR_2 7

bool driveDir = false;
bool bit_alarm = false;
int IR_baypass=0;
// bool bit_alarm_ok = false;

 #define speed_25 124
 #define speed_50 168
 #define speed_75 211
 #define speed_100 230

 unsigned int speed_summ, count_speed, res_speed;
/*
 float speed_summ =0.0;
  float count_speed =0.0;
   float res_speed =0.0;
 */  

int task_motor =0;
int Stbay_pwm_colagen;
int brightness = 20;
int fadeAmount = 5;
int Free_Program, recept;
int sessionPauseCntDwn = 0;

int Home_Page=0;

int ss=59, mm=48, count_div_1s=0, position_ss, position_mm, start_time;
int mm_3min, ss_3min;
int Revers_mm, Revers_ss;
int Position=0;
//int End_Session=0;

//on  5A A5 06 83 50 20 01 00 01 
//off 5A A5 06 83 50 20 01 00 00 

uint8_t Button_Setting[] = {0x5A, 0xA5, 0x06, 0x83, 0x54, 0x68, 0x01, 0x00, 0x01}; // Setting 
uint8_t Button_Sensor_ON[] = {0x5A, 0xA5, 0x06, 0x83, 0x54, 0x90, 0x01, 0x00, 0x01}; // Button_Sensor_ON   
uint8_t Button_Sensor_OFF[] = {0x5A, 0xA5, 0x06, 0x83, 0x54, 0x90, 0x01, 0x00, 0x00}; // Button_Sensor_OFF  
uint8_t Button_Sensor_Save[] = {0x5A, 0xA5, 0x06, 0x83, 0x54, 0x62, 0x01, 0x00, 0x01}; // Button_Sensor_Save    

uint8_t Start_Screen1[9] = {0x5A, 0xA5, 0x06, 0x83, 0x54, 0x36, 0x01, 0x00, 0x01}; //старт при нажатии  главного экрана 
uint8_t Back[9] = {0x5A, 0xA5, 0x06, 0x83, 0x54, 0x38, 0x01, 0x00, 0x01}; //кнопка назад
uint8_t AvtoSpeed_Session_ON[9] = {0x5A, 0xA5, 0x06, 0x83, 0x54, 0x46, 0x01, 0x00, 0x01}; // AvtoSpeed_Session_ON            
uint8_t AvtoSpeed_Session_OFF[9] = {0x5A, 0xA5, 0x06, 0x83, 0x54, 0x46, 0x01, 0x00, 0x00}; // AvtoSpeed_Session_OFF     

uint8_t AvtoSpeed_Page1_ON[9] = {0x5A, 0xA5, 0x06, 0x83, 0x50, 0x20, 0x01, 0x00, 0x01}; // AvtoSpeed_Page1_ON            
uint8_t AvtoSpeed_Page1_OFF[9] = {0x5A, 0xA5, 0x06, 0x83, 0x50, 0x20, 0x01, 0x00, 0x00}; // AvtoSpeed_Page1_OFF

uint8_t Start[9] = {0x5A, 0xA5, 0x06, 0x83, 0x54, 0x40, 0x01, 0x00, 0x01}; // Start
uint8_t Stop[9] = {0x5A, 0xA5, 0x06, 0x83, 0x54, 0x42, 0x01, 0x00, 0x01}; // Stop     5a a5 06 83 54 42 01 00 01            

uint8_t Session_30min[9] = {0x5A, 0xA5, 0x06, 0x83, 0x54, 0x70, 0x01, 0x00, 0x01}; // Session_30min
uint8_t Session_45min[9] = {0x5A, 0xA5, 0x06, 0x83, 0x54, 0x70, 0x01, 0x00, 0x02}; // Session_45min  
uint8_t Session_60min[9] = {0x5A, 0xA5, 0x06, 0x83, 0x54, 0x70, 0x01, 0x00, 0x03}; // Session_60min

uint8_t Session_Free[9] = {0x5A, 0xA5, 0x06, 0x83, 0x54, 0x70, 0x01, 0x00, 0x04}; // Session_Free
uint8_t Session_5min[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x02, 0x01, 0x00, 0x00}; // Session_5min 
uint8_t Session_10min[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x02, 0x01, 0x00, 0x01}; // Session_10min
uint8_t Session_15min[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x02, 0x01, 0x00, 0x02}; // Session_15min
uint8_t Session_20min[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x02, 0x01, 0x00, 0x03}; // Session_20min
uint8_t Session_25min[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x02, 0x01, 0x00, 0x04}; // Session_25min
uint8_t Session_30minF[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x02, 0x01, 0x00, 0x05}; // Session_30min
uint8_t Session_35min[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x02, 0x01, 0x00, 0x06}; // Session_35min
uint8_t Session_40min[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x02, 0x01, 0x00, 0x07}; // Session_40min
uint8_t Session_45minF[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x02, 0x01, 0x00, 0x08}; // Session_45min
uint8_t Session_50min[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x02, 0x01, 0x00, 0x09}; // Session_50min
uint8_t Session_55min[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x02, 0x01, 0x00, 0x0a}; // Session_55min
uint8_t Session_60minF[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x02, 0x01, 0x00, 0x0b}; // Session_60min

// uint8_t Session_speed_0[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x00, 0x01, 0x00, 0x00}; // Session_speed_0
uint8_t Session_speed_25[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x00, 0x01, 0x00, 0x00}; // Session_speed_25
uint8_t Session_speed_50[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x00, 0x01, 0x00, 0x01}; // Session_speed_50
uint8_t Session_speed_75[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x00, 0x01, 0x00, 0x02}; // Session_speed_75
uint8_t Session_speed_100[9] = {0x5A, 0xA5, 0x06, 0x83, 0x62, 0x00, 0x01, 0x00, 0x03}; // Session_speed_100

uint8_t Session_Rot_ON[9] = {0x5A, 0xA5, 0x06, 0x83, 0x50, 0x00, 0x01, 0x00, 0x01}; // Session_Rot_ON
uint8_t Session_Rot_OFF[9] = {0x5A, 0xA5, 0x06, 0x83, 0x50, 0x00, 0x01, 0x00, 0x00}; // Session_Rot_OFF
uint8_t Session_Reset[9] = {0x5A, 0xA5, 0x06, 0x83, 0x54, 0x44, 0x01, 0x00, 0x01}; // Session_Reset 

uint8_t Colagen_ON[9] = {0x5A, 0xA5, 0x06, 0x83, 0x50, 0x04, 0x01, 0x00, 0x01}; // Colagen_ON      
uint8_t Colagen_OFF[9] = {0x5A, 0xA5, 0x06, 0x83, 0x50, 0x04, 0x01, 0x00, 0x00}; // Colagen_OFF      

uint8_t Infra_ON[9] = {0x5A, 0xA5, 0x06, 0x83, 0x50, 0x02, 0x01, 0x00, 0x01}; // Infra_ON      
uint8_t Infra_OFF[9] = {0x5A, 0xA5, 0x06, 0x83, 0x50, 0x02, 0x01, 0x00, 0x00}; // Infra_OFF  

/*
5a a5 05 82 57 00 00 00  - позиция №1
5a a5 05 82 57 00 00 01 - позиция №2
5a a5 05 82 57 00 00 02 - позиция №3
5a a5 05 82 57 00 00 03 - позиция №4
5a a5 05 82 57 00 00 04 - позиция №5
5a a5 05 82 57 00 00 05 - позиция №6
5a a5 05 82 57 00 00 06 - позиция №7
5a a5 05 82 57 00 00 07 - позиция №8
5a a5 05 82 57 00 00 08 - позиция №9
5a a5 05 82 57 00 00 09 - позиция №10
5a a5 05 82 57 00 00 0a - позиция №11
5a a5 05 82 57 00 00 0b - позиция №12
5a a5 05 82 57 00 00 0c - позиция №13
5a a5 05 82 57 00 00 0d - позиция №14
5a a5 05 82 57 00 00 0e - позиция №15
*/
uint8_t Transmit_Image_1[8] = {0x5A, 0xA5, 0x05, 0x82, 0x57, 0x00, 0x00, 0x00}; // Image_1
uint8_t Transmit_Image_2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x57, 0x00, 0x00, 0x01}; // Image_2
uint8_t Transmit_Image_3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x57, 0x00, 0x00, 0x02}; // Image_1
uint8_t Transmit_Image_4[8] = {0x5A, 0xA5, 0x05, 0x82, 0x57, 0x00, 0x00, 0x03}; // Image_1
uint8_t Transmit_Image_5[8] = {0x5A, 0xA5, 0x05, 0x82, 0x57, 0x00, 0x00, 0x04}; // Image_1
uint8_t Transmit_Image_6[8] = {0x5A, 0xA5, 0x05, 0x82, 0x57, 0x00, 0x00, 0x05}; // Image_1
uint8_t Transmit_Image_7[8] = {0x5A, 0xA5, 0x05, 0x82, 0x57, 0x00, 0x00, 0x06}; // Image_1
uint8_t Transmit_Image_8[8] = {0x5A, 0xA5, 0x05, 0x82, 0x57, 0x00, 0x00, 0x07}; // Image_1
uint8_t Transmit_Image_9[8] = {0x5A, 0xA5, 0x05, 0x82, 0x57, 0x00, 0x00, 0x08}; // Image_1
uint8_t Transmit_Image_10[8] = {0x5A, 0xA5, 0x05, 0x82, 0x57, 0x00, 0x00, 0x09}; // Image_1
uint8_t Transmit_Image_11[8] = {0x5A, 0xA5, 0x05, 0x82, 0x57, 0x00, 0x00, 0x0a}; // Image_1
uint8_t Transmit_Image_12[8] = {0x5A, 0xA5, 0x05, 0x82, 0x57, 0x00, 0x00, 0x0b}; // Image_1
uint8_t Transmit_Image_13[8] = {0x5A, 0xA5, 0x05, 0x82, 0x57, 0x00, 0x00, 0x0c}; // Image_1
uint8_t Transmit_Image_14[8] = {0x5A, 0xA5, 0x05, 0x82, 0x57, 0x00, 0x00, 0x0d}; // Image_1
uint8_t Transmit_Image_15[8] = {0x5A, 0xA5, 0x05, 0x82, 0x57, 0x00, 0x00, 0x0e}; // Image_1

//отправка
uint8_t Transmit_Page_Session[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x05}; // без автоскорости
uint8_t Transmit_Page_Seeeion_avto[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x06}; // с автоскоростью
uint8_t Transmit_Page_Free[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5a, 0x01, 0x00, 0x11}; // Transmit_Page_Free 5a a5 07 82 00 84 5a 01 00 01
uint8_t Transmit_Page_Result[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5a, 0x01, 0x00, 0x08}; //окно результата
uint8_t Transmit_Alarm[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5a, 0x01, 0x00, 0x0a}; //Alarm 5a a5 07 82 00 84 5a 01 00 04
uint8_t Transmit_Page_Start[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5a, 0x01, 0x00, 0x01};

uint8_t Transmit_Speed_25[] = {0x5A, 0xA5, 0x05, 0x82, 0x62, 0x00, 0x00, 0x00}; //  скорость 25 
uint8_t Transmit_Speed_50[] = {0x5A, 0xA5, 0x05, 0x82, 0x62, 0x00, 0x00, 0x01}; //  скорость 50
uint8_t Transmit_Speed_75[] = {0x5A, 0xA5, 0x05, 0x82, 0x62, 0x00, 0x00, 0x02}; //  скорость 75 
uint8_t Transmit_Speed_100[] = {0x5A, 0xA5, 0x05, 0x82, 0x62, 0x00, 0x00, 0x03}; //  скорость 100 
uint8_t Transmit_Reset[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x04, 0x55, 0xaa, 0x5a, 0xa5}; //   сброс дисплея
uint8_t Transmit_Sensor_ON[] = {0x5A, 0xA5, 0x05, 0x82, 0x54, 0x90, 0x00, 0x01}; //Transmit_Sensor_ON кнопка 
uint8_t Transmit_Sensor_OFF[] = {0x5A, 0xA5, 0x05, 0x82, 0x54, 0x90, 0x00, 0x00}; //Transmit_Sensor_OFF кнопка 

bool bit_AvtoSpeed = true; //начальное сосотяние атоскорости OFF
bool bit_Start = false; //начальное сосотяние OFF
bool bit_Start_Time = false;
bool bit_St_Bay = true; //бит St_Bay 
int count_Dec_Sart=0; //счетчик при начале серии 3,2,1

int calories, Div_calories, Div_program;

unsigned int counter_position;

//45/15
//60/15

#define recipe_pos_1 180  // 180сек
#define recipe_pos_2 365//360 5
#define recipe_pos_3 550//540 10
#define recipe_pos_4 735//720 15
#define recipe_pos_5 920 //900 20
#define recipe_pos_6 1105 //1080 25
#define recipe_pos_7 1290 //1260 30
#define recipe_pos_8 1475 //1440 35
#define recipe_pos_9 1660 //1620 40
#define recipe_pos_10 1845 //1800 45
#define recipe_pos_11 2030 //
#define recipe_pos_12 2215 //
#define recipe_pos_13 2400 //
#define recipe_pos_14 2585 //
#define recipe_pos_15 2770 //

#define recipe3_pos_1 240
#define recipe3_pos_2 485//
#define recipe3_pos_3 730
#define recipe3_pos_4 975
#define recipe3_pos_5 1220
#define recipe3_pos_6 1465
#define recipe3_pos_7 1710
#define recipe3_pos_8 1955
#define recipe3_pos_9 2200
#define recipe3_pos_10 2445
#define recipe3_pos_11 2690
#define recipe3_pos_12 2935
#define recipe3_pos_13 3180
#define recipe3_pos_14 3425
#define recipe3_pos_15 3670

//==== MILLISTIMER MACRO ==== //удобный таймер на millis()
#define EVERY_MS(x) \
  static uint32_t tmr; \
  bool flag = millis() - tmr >= (x); \
  if (flag) tmr += (x); \
  if (flag)


class Class_privod {     // 
  public:
    void privod_start();
    void privod_stop();
    void privod_task(int n);
    void privod_back();
    void privod_forward();
    void privod_rele_start();
    void privod_rele_stop();
  private:
};

void Class_privod::privod_start ()
{  
     digitalWrite (pin_start, HIGH);
}

void Class_privod::privod_stop ()
{  
    digitalWrite (pin_start, LOW);
}

void Class_privod::privod_task (int n)
{  
    analogWrite (task,n);
}

void Class_privod::privod_back ()
{  
    driveDir = HIGH;
}

void Class_privod::privod_forward ()
{  
    driveDir = LOW;
}

void Class_privod::privod_rele_stop ()
{  
    digitalWrite (rele_cap, LOW);
    digitalWrite (rele_start, LOW);
}

void Class_privod::privod_rele_start ()
{  
  digitalWrite (rele_cap, HIGH);
  digitalWrite (rele_start, HIGH); 
}

Class_privod privod;


void USART_Transmit(unsigned char data) {
  // Ожидание освобождения буфера передатчика
  while (!(UCSR0A & (1 << UDRE0)));
  // Запись данных в буфер передатчика и отправка
  UDR0 = data;
}

void USART_TransmitString(const char* str) {
  for (int i = 0; str[i] != '\0'; i++) {
    USART_Transmit(str[i]);
  }
}

void USART_init(void) {
  UBRR0H = (MYUBRR >> 8);
  UBRR0L = MYUBRR;
  UCSR0B |= (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
  UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
}


void setup() {
  USART_init();
  pinMode(led_colagen, OUTPUT);
  pinMode(back, OUTPUT); 
  digitalWrite (back, LOW);
  pinMode(pin_start, OUTPUT); 
  digitalWrite (pin_start, LOW);
  pinMode(task, OUTPUT); 
  pinMode(rele_start, OUTPUT); 
  pinMode(rele_cap, OUTPUT); 
  pinMode(lamp, OUTPUT); 
  digitalWrite (lamp, LOW);
  pinMode(led_colagen, OUTPUT); 
  digitalWrite (led_colagen, LOW);
  pinMode(IR_1, INPUT_PULLUP);
  pinMode(IR_2, INPUT_PULLUP);
  IR_baypass = EEPROM.read(0); //Button_IR_ON
  privod.privod_stop();
  Home_Page=1; //при вкл мы на домашней траницу
}

void loop() {

  EVERY_MS(100) {

    if (sessionPauseCntDwn == 0) {

      static uint32_t cnt = 0;

      if ((digitalRead(IR_1) == HIGH || digitalRead(IR_2) == HIGH)) {
        cnt++;
      }
      else {
        cnt = 0;
      }
// if ( (IR_baypass == 0x3a) || ((digitalRead(IR_1) == LOW && digitalRead(IR_2) == LOW)) ) {
      if ( (IR_baypass == 0x3a) || (cnt < 5 ) ) {
        
        if (bit_alarm) {
          bit_alarm = false;
          actualize_a_page();
        }

        session();
      }
      else {
        privod.privod_rele_stop();
        privod.privod_stop();
        privod.privod_task(0);
        ALARMS();
        bit_alarm = true;
      }
    }
    else {
      sessionPauseCntDwn--;
    }
  }
}


//Обратное время для таблицы результата 
void Revers_Time() 
{
  if (Revers_ss < 59) {
    Revers_ss++;
  }
  else {
  if (Revers_mm < 59) {
    Revers_mm++;
    Revers_ss = 0;
    } else { }
  }
}


void total_time (void)
{
  if (bit_Start_Time == true) {
    
    if (Div_calories<10) {
      Div_calories++;
    }
    else {
      func_calories();
      Div_calories=0;
    }
    
    if (ss > 0) {
      ss--;
    } else {
      if (mm > 0) {
        mm--;
        ss = 59;
      } else { }
    }

    if (ss == 0 && mm == 0) {
      stop_session();
    }
    else {
      out_Displey_Time(mm,ss);
      Revers_Time();
    }
    // второй таймер
    if (recept != 4 ) { 
      if (ss_3min > 0) {
        ss_3min--;
      }
      else {
        if (mm_3min > 0) {
          mm_3min--;
          ss_3min = 59;
        }
        else {
          if (recept == 1 || recept == 2) {
            mm_3min = 2;
            ss_3min = 59;
          }
          if (recept == 3) {
            mm_3min = 3;
            ss_3min = 59;
          }
        }
      }
    }
    //5a a5 05 82 64 04 03 09
    USART_Transmit(0x5A); USART_Transmit(0xA5); USART_Transmit(0x05); USART_Transmit(0x82); USART_Transmit(0x64); USART_Transmit(0x04);
    uint8_t mmHighNibble_3min = (mm_3min >> 4) & 0x0F;
    uint8_t mmLowNibble_3min = mm_3min & 0x0F;
    uint8_t ssHighNibble_3min = (ss_3min >> 4) & 0x0F;
    uint8_t ssLowNibble_3min = ss_3min & 0x0F;
    char hexMM_3min[3]; // Буфер для шестнадцатеричного представления минут
    char hexSS_3min[3]; // Буфер для шестнадцатеричного представления секунд
    // Конвертируем mm в шестнадцатеричный формат и сохраняем в hexMM
    sprintf(hexMM_3min, "%02X", mm_3min);
    // Конвертируем ss в шестнадцатеричный формат и сохраняем в hexSS
    sprintf(hexSS_3min, "%02X", ss_3min);
    int intMM_3min, intSS_3min;
    // Конвертируем шестнадцатеричное представление обратно в целочисленные значения
    sscanf(hexMM_3min, "%X", &intMM_3min);
    sscanf(hexSS_3min, "%X", &intSS_3min);
    USART_Transmit(intMM_3min); USART_Transmit(intSS_3min);
  }
}


void out_Displey_Time(int mmm, int sss)
{
  //
  USART_Transmit(0x5A); USART_Transmit(0xA5); USART_Transmit(0x05); USART_Transmit(0x82); USART_Transmit(0x64); USART_Transmit(0x02); //USART_Transmit(0x03); USART_Transmit(0x0B);
  uint8_t mmHighNibble = (mmm >> 4) & 0x0F;
  uint8_t mmLowNibble = mmm & 0x0F;
  uint8_t ssHighNibble = (sss >> 4) & 0x0F;
  uint8_t ssLowNibble = sss & 0x0F;
  //
  char hexMM[3]; // Буфер для шестнадцатеричного представления минут
  char hexSS[3]; // Буфер для шестнадцатеричного представления секунд
  // Конвертируем mm в шестнадцатеричный формат и сохраняем в hexMM
  sprintf(hexMM, "%02X", mmm);
  // Конвертируем ss в шестнадцатеричный формат и сохраняем в hexSS
  sprintf(hexSS, "%02X", sss);
  int intMM, intSS;
  // Конвертируем шестнадцатеричное представление обратно в целочисленные значения
  sscanf(hexMM, "%X", &intMM);
  sscanf(hexSS, "%X", &intSS);
  USART_Transmit(intMM); USART_Transmit(intSS);
}

//Калории = (200 ккал/час) * (Скорость / 100) * (Время / 60)
void func_calories ()
{
  calories++;
  uint8_t cal1 = (calories >> 8) & 0xFF; // Старший байт
  uint8_t cal2 = calories & 0xFF;        // Младший байт

  char hexCal1[3]; // Буфер для шестнадцатеричного представления старшего байта
  char hexCal2[3]; // Буфер для шестнадцатеричного представления младшего байта

  // Конвертируем cal1 в шестнадцатеричный формат и сохраняем в hexCal1
  sprintf(hexCal1, "%02X", cal1);
  // Конвертируем cal2 в шестнадцатеричный формат и сохраняем в hexCal2
  sprintf(hexCal2, "%02X", cal2);

  int intCal1, intCal2;
  // Конвертируем шестнадцатеричное представление обратно в целочисленные значения
  sscanf(hexCal1, "%X", &intCal1);
  sscanf(hexCal2, "%X", &intCal2);

  USART_Transmit(0x5A); USART_Transmit(0xA5); USART_Transmit(0x05); USART_Transmit(0x82); USART_Transmit(0x64); USART_Transmit(0x00); 
  USART_Transmit(intCal1); USART_Transmit(intCal2);
}


void pos ()
{
  USART_Transmit(0x5A); USART_Transmit(0xA5); USART_Transmit(0x05); USART_Transmit(0x82); USART_Transmit(0x64); USART_Transmit(0x08); USART_Transmit(0x00); USART_Transmit(0x04);
}


void Displey_speed_25()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Speed_25[i]); }
}

void Displey_speed_50()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Speed_50[i]); }
}

void Displey_speed_75()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Speed_75[i]); }
}

void Displey_speed_100()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Speed_100[i]); }
}


void Speed_result()
{
  USART_Transmit(0x5a); USART_Transmit(0xa5); USART_Transmit(0x05); USART_Transmit(0x82); USART_Transmit(0x64); USART_Transmit(0x10); USART_Transmit(0x00); //USART_Transmit(0x09);
  count_speed++;
  speed_summ=(speed_summ/count_speed); //count_speed
  res_speed=speed_summ;

  char hexCal[5];
  sprintf(hexCal, "0x%02X", res_speed);
  int transmittedValue = strtol(hexCal, NULL, 0); // Преобразование строки в число
  USART_Transmit(transmittedValue);
}


void actualize_a_page (void)
{
  if (bit_Start == true) {
    if (recept != 4) {
      if (bit_AvtoSpeed == true) {
        Page_Session_Auto();
      }
      else {
        Page_Session();
      }
    }
    else {
      Page_Free();
    }
  }
  else {
    Page_Start();
  }
}


void session (void)
{
  if (count_div_1s < 10) {
    count_div_1s++;
  }
  else {
    count_div_1s = 0;
    if (start_time == true) {
      total_time();
    }
  }

  digitalWrite (back, driveDir);

  if (bit_Start == true) {

    if (count_Dec_Sart != 70)
      count_Dec_Sart++;
          
    if (count_Dec_Sart == 68) {

      speed_summ = 50;

      actualize_a_page();
        
      Image_1();
      bit_Start_Time = true;
      Displey_speed_50();
    }
            
    if (count_Dec_Sart == 70) {
      ////////////////////////recept
      if (Div_program < 10) {
        Div_program++;
      }
      else {
        Div_program=0;

        if (recept==1)
          program_1();
        if (recept==2)
          program_2();
        if (recept==3)
          program_3();
        if (recept==4)
          program_4();
      }
    }
  }
  else {
    privod.privod_rele_stop();
    privod.privod_stop();
    privod.privod_task(0);
  }
}


void  stop_session ()
{
  bit_Start=false;
  bit_St_Bay=true;  
  count_Dec_Sart=0;
  bit_Start_Time=false;
  calories=0;
  counter_position=0;
  bit_AvtoSpeed=false;
  Div_program=0;
  recept=0;
  analogWrite(led_colagen, 0);
  digitalWrite (lamp, LOW);
  position_ss=0;
  position_mm=0;
  start_time=false;
  privod.privod_forward();

  for (uint8_t i = 0; i < 10; i++) {
    USART_Transmit(Transmit_Page_Result[i]);
  }
    out_Displey_Time(Revers_mm,Revers_ss);
    Position_Res(Position);
    Speed_result(); 
}


void PARSING(uint8_t* data) {
  // проверяем деружный режим, когда находимся на 1 экране, то перелиается коллаген, когда на 2м экране все выкл переход в режим готовности
  if (memcmp(data, Start_Screen1, 9) == 0) {  bit_St_Bay=false;   Home_Page=0; } 
  if (memcmp(data, Back, 9) == 0) { bit_St_Bay=true;  } 

  if (memcmp(data, Button_Setting, 9) == 0) {
    if(IR_baypass == 0x3a) { Button_IR_OFF(); }
    else {Button_IR_ON();  } 
  }
  if (memcmp(data, AvtoSpeed_Session_ON, 9) == 0) { bit_AvtoSpeed=true; }
  if (memcmp(data, AvtoSpeed_Session_OFF, 9) == 0) {bit_AvtoSpeed=false;}

  if (memcmp(data, AvtoSpeed_Page1_ON, 9) == 0) { bit_AvtoSpeed=true; }
  if (memcmp(data, AvtoSpeed_Page1_OFF, 9) == 0) { bit_AvtoSpeed=false; }

  if (memcmp(data, Session_30min, 9) == 0) { bit_AvtoSpeed = 1; mm=29; ss=59;  Free_Program=0; recept=1; mm_3min = 2; ss_3min = 59;  }
  if (memcmp(data, Session_45min, 9) == 0) { bit_AvtoSpeed = 1; mm=44; ss=59;  Free_Program=0; recept=2; mm_3min = 2; ss_3min = 59;  }
  if (memcmp(data, Session_60min, 9) == 0) { bit_AvtoSpeed = 1; mm=59; ss=59;  Free_Program=0; recept=3; mm_3min = 3; ss_3min = 59;  }
  if (memcmp(data, Session_Free, 9) == 0)  { /*mm=29; ss=59;*/  Free_Program=1; recept=4;  }       //было 4.58                          

  if (memcmp(data, Session_5min, 9) == 0) { mm=4; ss=59; recept=4; }           
  if (memcmp(data, Session_10min, 9) == 0) { mm=9; ss=59; recept=4; }
  if (memcmp(data, Session_15min, 9) == 0) { mm=14; ss=59; recept=4; }
  if (memcmp(data, Session_20min, 9) == 0) { mm=19; ss=59; recept=4; }
  if (memcmp(data, Session_25min, 9) == 0) { mm=24; ss=59; recept=4; }
  if (memcmp(data, Session_30minF, 9) == 0) { mm=29; ss=59; recept=4; }
  if (memcmp(data, Session_35min, 9) == 0) { mm=34; ss=59; recept=4; }
  if (memcmp(data, Session_40min, 9) == 0) { mm=39; ss=59; recept=4; }
  if (memcmp(data, Session_45minF, 9) == 0) { mm=44; ss=59; recept=4; }
  if (memcmp(data, Session_50min, 9) == 0) { mm=49; ss=59; recept=4; }
  if (memcmp(data, Session_55min, 9) == 0) { mm=54; ss=59; recept=4; }
  if (memcmp(data, Session_60minF, 9) == 0) { mm=59; ss=59; recept=4; }

  if (memcmp(data, Colagen_ON, 9) == 0) { analogWrite(led_colagen, 100); }
  if (memcmp(data, Colagen_OFF, 9) == 0) { analogWrite(led_colagen, 0); }

  if (memcmp(data, Infra_ON, 9) == 0) { digitalWrite (lamp, HIGH); }
  if (memcmp(data, Infra_OFF, 9) == 0) { digitalWrite (lamp, LOW); }
  
  // if (memcmp(data, Session_speed_0, 9) == 0) { task_motor=speed_25; 
  //     USART_Transmit(0x5A); USART_Transmit(0xA5); USART_Transmit(0x05); USART_Transmit(0x82); USART_Transmit(0x62);
  //     USART_Transmit(0x00); USART_Transmit(0x00); USART_Transmit(0x01);
  // }
  if (memcmp(data, Session_speed_25, 9) == 0) { task_motor=speed_25; speed_summ=(speed_summ+25); count_speed++; }
  if (memcmp(data, Session_speed_50, 9) == 0) { task_motor=speed_50; speed_summ=(speed_summ+50); count_speed++; }
  if (memcmp(data, Session_speed_75, 9) == 0) { task_motor=speed_75; speed_summ=(speed_summ+75); count_speed++; }
  if (memcmp(data, Session_speed_100, 9) == 0) { task_motor=speed_100; speed_summ=(speed_summ+100); count_speed++; }

  if (memcmp(data, Session_Rot_ON, 9) == 0) {
    privod.privod_task(0);
    privod.privod_rele_stop();
    sessionPauseCntDwn = 5;
    privod.privod_back();
  }

  if (memcmp(data, Session_Rot_OFF, 9) == 0) {
    privod.privod_task(0);
    privod.privod_rele_stop();
    sessionPauseCntDwn = 5;
    privod.privod_forward();
  }

  if (memcmp(data, Start, 9) == 0) {  
    if ( Free_Program && mm != 4 && mm != 9 && mm != 14 && mm!=19 && mm != 24 && mm != 34 && mm != 39 && mm != 44 && mm !=
                                                                                              49 && mm != 54 && mm != 59) {
      mm=29; ss=59;
    }
    bit_Start = true;
    task_motor=speed_50;
    start_time=true;
  }
  
  if (memcmp(data, Stop, 9) == 0) {  stop_session();  }
  if (memcmp(data, Session_Reset, 9) == 0) { Revers_ss=0; Revers_mm=0; speed_summ=0; count_speed=0; res_speed=0; ss=59; mm=29;   RESET_DISP(); }

  if (memcmp(data, Button_Sensor_ON, 9) == 0) { IR_baypass=255;  }   
  if (memcmp(data, Button_Sensor_OFF, 9) == 0) { IR_baypass=0x3a;  }   
  if (memcmp(data, Button_Sensor_Save, 9) == 0) { EEPROM.write(0, IR_baypass);  }
}

 

ISR(USART_RX_vect) {

  static uint8_t receivedBytes[9];
  static uint8_t receivedIndex = 0;
  char receivedByte = UDR0;

  // Проверяем, является ли полученный байт началом строки (0x5A)
  if (receivedByte == 0x5A) {
    // Сбрасываем receivedIndex в 0 и начинаем сохранять данные в буфер receivedBytes
    receivedIndex = 0;
  }

  // Если начало строки уже обнаружено, продолжаем сохранять остальные байты
  if (receivedIndex < 9) {
    receivedBytes[receivedIndex++] = receivedByte;
  }

  // Проверяем, было ли получено 9 байтов (включая первый байт 0x5A)
  if (receivedIndex == 9) {
    // Вызываем функцию PARSING() и передаем ей принятые данные
    PARSING(receivedBytes);
    // Сбрасываем receivedIndex в 0 для приема следующей строки
    receivedIndex = 0;
  }
}


void Position_Res (int pos)
{
  USART_Transmit(0x5A); USART_Transmit(0xA5); USART_Transmit(0x05); USART_Transmit(0x82); USART_Transmit(0x64); USART_Transmit(0x08); USART_Transmit(0x00);
  if(pos==0) {USART_Transmit(0x00); }
  if(pos==1) {USART_Transmit(0x01); }  
  if(pos==2) {USART_Transmit(0x02); }
  if(pos==3) {USART_Transmit(0x03); }
  if(pos==4) {USART_Transmit(0x04); }
  if(pos==5) {USART_Transmit(0x05); }
  if(pos==6) {USART_Transmit(0x06); }
  if(pos==7) {USART_Transmit(0x07); }
  if(pos==8) {USART_Transmit(0x08); }
  if(pos==9) {USART_Transmit(0x09); }
  if(pos==10) {USART_Transmit(0x0a); }
  if(pos==11) {USART_Transmit(0x0b); }
  if(pos==12) {USART_Transmit(0x0c); }
  if(pos==13) {USART_Transmit(0x0d); }
  if(pos==14) {USART_Transmit(0x0e); }
  if(pos==15) {USART_Transmit(0x0f); }
}

void RESET_DISP ()
{
for (uint8_t i = 0; i < 10; i++) { USART_Transmit(Transmit_Reset[i]); }
Home_Page=1; Position=0;
}

void ALARMS ()
{
  for (uint8_t i = 0; i < 10; i++) { USART_Transmit(Transmit_Alarm[i]); }
}


void Image_1 ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Image_1[i]); }
}

void Image_2 ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Image_2[i]); }
}

void Image_3 ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Image_3[i]); }
}

void Image_4 ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Image_4[i]); }
}

void Image_5 ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Image_5[i]); }
}

void Image_6 ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Image_6[i]); }
}

void Image_7 ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Image_7[i]); }
}

void Image_8 ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Image_8[i]); }
}

void Image_9 ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Image_9[i]); }
}

void Image_10 ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Image_10[i]); }
}

void Image_11 ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Image_11[i]); }
}

void Image_12 ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Image_12[i]); }
}

void Image_13 ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Image_13[i]); }
}

void Image_14 ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Image_14[i]); }
}

void Image_15 ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Image_15[i]); }
}

void Button_IR_ON ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Sensor_ON[i]); }
}

void Button_IR_OFF ()
{
  for (uint8_t i = 0; i < 8; i++) { USART_Transmit(Transmit_Sensor_OFF[i]); }
}

void Page_Free ()
{
  for (uint8_t i = 0; i < 10; i++) { USART_Transmit(Transmit_Page_Free[i]); }
}

void Page_Session ()
{
  for (uint8_t i = 0; i < 10; i++) { USART_Transmit(Transmit_Page_Session[i]); }
}

void Page_Session_Auto ()
{
  for (uint8_t i = 0; i < 10; i++) { USART_Transmit(Transmit_Page_Seeeion_avto[i]); }
}

void Page_Start ()
{
  for (uint8_t i = 0; i < 10; i++) { USART_Transmit(Transmit_Page_Start[i]); }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PROGRAM 1
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void program_1 ()
{       
        counter_position++;
       // privod.privod_start();
      //  privod.privod_rele_start();
      //  privod.privod_task(task_motor);

        //Позиция 1
    if(counter_position < recipe_pos_1) 
      { 
        Position=1;
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_1();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_50); Displey_speed_50(); task_motor=speed_50; }
      }
//Позиция 2
    if(counter_position == recipe_pos_1-1)
     {
      Position=2;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_2 && counter_position > recipe_pos_1+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_2();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_25); Displey_speed_25(); task_motor=speed_25; }
     }
//Позиция 3
    if(counter_position == recipe_pos_2-1)
     {
      Position=3;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_3 && counter_position > recipe_pos_2+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_3();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_75); Displey_speed_75(); task_motor=speed_75; }
     }
//Позиция 4
    if(counter_position == recipe_pos_3-1)
     {
      Position=4;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_4 && counter_position > recipe_pos_3+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_4();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_25); Displey_speed_25(); task_motor=speed_25; }
     }   
//Позиция 5
    if(counter_position == recipe_pos_4-1)
     {
      Position=5;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_5 && counter_position > recipe_pos_4+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_5();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_50); Displey_speed_50(); task_motor=speed_50; }
     }    
//Позиция 6
    if(counter_position == recipe_pos_5-1)
     {
      Position=6;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_6 && counter_position > recipe_pos_5+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_6();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_100); Displey_speed_100(); task_motor=speed_100; }
     }   

//Позиция 7
    if(counter_position == recipe_pos_6-1)
     {
      Position=7;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_7 && counter_position > recipe_pos_6+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_7();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_50); Displey_speed_50(); task_motor=speed_50; }
     }   
//Позиция 8
    if(counter_position == recipe_pos_7-1)
     {
      Position=8;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_8 && counter_position > recipe_pos_7+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_8();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_25); Displey_speed_25(); task_motor=speed_25; }
     }   
//Позиция 9
    if(counter_position == recipe_pos_8-1)
     {
      Position=9;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_9 && counter_position > recipe_pos_8+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_9();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_75); Displey_speed_75(); task_motor=speed_75; }
     } 
//Позиция 10
    if(counter_position == recipe_pos_9-1)
     {
      Position=10;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_10 && counter_position > recipe_pos_9+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_10();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_100); Displey_speed_100(); task_motor=speed_100; }
     }                               
//      
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//END PROGRAM 1
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PROGRAM 2
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void program_2 ()
{       
        counter_position++;
        
        //Позиция 1
    if(counter_position < recipe_pos_1) 
      { 
        Position=1;
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_1();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_50); Displey_speed_50(); task_motor=speed_50; }
      }
//Позиция 2
    if(counter_position == recipe_pos_1-1)
     {
      Position=2;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_2 && counter_position > recipe_pos_1+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_2();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_25); Displey_speed_25(); task_motor=speed_25; }
     }
//Позиция 3
    if(counter_position == recipe_pos_2-1)
     {
      Position=3;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_3 && counter_position > recipe_pos_2+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_3();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_75); Displey_speed_75(); task_motor=speed_75; }
     }
//Позиция 4
    if(counter_position == recipe_pos_3-1)
     {
      Position=4;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_4 && counter_position > recipe_pos_3+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_4();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_25); Displey_speed_25(); task_motor=speed_25; }
     }   
//Позиция 5
    if(counter_position == recipe_pos_4-1)
     {
      Position=5;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_5 && counter_position > recipe_pos_4+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_5();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_50); Displey_speed_50(); task_motor=speed_50; }
     }    
//Позиция 6
    if(counter_position == recipe_pos_5-1)
     {
      Position=6;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_6 && counter_position > recipe_pos_5+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_6();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_100); Displey_speed_100(); task_motor=speed_100; }
     }   

//Позиция 7
    if(counter_position == recipe_pos_6-1)
     {
      Position=7;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_7 && counter_position > recipe_pos_6+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_7();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_50); Displey_speed_50(); task_motor=speed_50; }
     }   
//Позиция 8
    if(counter_position == recipe_pos_7-1)
     {
      Position=8;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_8 && counter_position > recipe_pos_7+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_8();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_25); Displey_speed_25(); task_motor=speed_25; }
     }   
//Позиция 9
    if(counter_position == recipe_pos_8-1)
     {
      Position=9;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_9 && counter_position > recipe_pos_8+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_9();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_75); Displey_speed_75(); task_motor=speed_75; }
     } 
//Позиция 10
    if(counter_position == recipe_pos_9-1)
     {
      Position=10;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_10 && counter_position > recipe_pos_9+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_10();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_100); Displey_speed_100(); task_motor=speed_100; }
     }  
//Позиция 11
    if(counter_position == recipe_pos_10-1)
     {
      Position=11;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_11 && counter_position > recipe_pos_10+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_11();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_50); Displey_speed_50(); task_motor=speed_50; }
     }   
//Позиция 12
    if(counter_position == recipe_pos_11-1)
     {
      Position=12;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_12 && counter_position > recipe_pos_11+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_12();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_25); Displey_speed_25(); task_motor=speed_25; }
     } 
//Позиция 13
    if(counter_position == recipe_pos_12-1)
     {
      Position=13;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_13 && counter_position > recipe_pos_12+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_13();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_25); Displey_speed_25(); task_motor=speed_25; }
     }     
 //Позиция 14
    if(counter_position == recipe_pos_13-1)
     {
      Position=14;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_14 && counter_position > recipe_pos_13+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_14();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_75); Displey_speed_75(); task_motor=speed_75; }
     }  
//Позиция 15
    if(counter_position == recipe_pos_14-1)
     {
      Position=15;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe_pos_15 && counter_position > recipe_pos_14+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_15();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_100); Displey_speed_100(); task_motor=speed_100; }
     }                                             
//      
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//END PROGRAM 1
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PROGRAM 3
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void program_3 ()
{ 
          counter_position++;
        
        //Позиция 1
    if(counter_position < recipe_pos_1) 
      { 
        Position=1;
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_1();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_50); Displey_speed_50(); task_motor=speed_50; }
      }
//Позиция 2
    if(counter_position == recipe3_pos_1-1)
     {
      Position=2;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe3_pos_2 && counter_position > recipe3_pos_1+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_2();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_25); Displey_speed_25(); task_motor=speed_25; }
     }
 //Позиция 3
    if(counter_position == recipe3_pos_2-1)
     {
      Position=3;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe3_pos_3 && counter_position > recipe3_pos_2+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_3();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_50); Displey_speed_50(); task_motor=speed_50; }
     }   
 //Позиция 4
    if(counter_position == recipe3_pos_3-1)
     {
      Position=4;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe3_pos_4 && counter_position > recipe3_pos_3+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_4();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_75); Displey_speed_75(); task_motor=speed_75; }
     }       
 //Позиция 5
    if(counter_position == recipe3_pos_4-1)
     {
      Position=5;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe3_pos_5 && counter_position > recipe3_pos_4+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_5();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_50); Displey_speed_50(); task_motor=speed_50; }
     } 
 //Позиция 6
    if(counter_position == recipe3_pos_5-1)
     {
      Position=6;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe3_pos_6 && counter_position > recipe3_pos_5+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_6();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_25); Displey_speed_25(); task_motor=speed_25; }
     }
 //Позиция 7
    if(counter_position == recipe3_pos_6-1)
     {
      Position=7;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe3_pos_7 && counter_position > recipe3_pos_6+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_7();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_100); Displey_speed_100(); task_motor=speed_100; }
     }  
 //Позиция 8
    if(counter_position == recipe3_pos_7-1)
     {
      Position=8;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe3_pos_8 && counter_position > recipe3_pos_7+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_8();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_50); Displey_speed_50(); task_motor=speed_50; }
     } 
 //Позиция 9
    if(counter_position == recipe3_pos_8-1)
     {
      Position=9;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe3_pos_9 && counter_position > recipe3_pos_8+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_9();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_75); Displey_speed_75(); task_motor=speed_75; }
     } 
 //Позиция 10
    if(counter_position == recipe3_pos_9-1)
     {
      Position=10;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe3_pos_10 && counter_position > recipe3_pos_9+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_10();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_25); Displey_speed_25(); task_motor=speed_25; }
     }   
  //Позиция 11
    if(counter_position == recipe3_pos_10-1)
     {
      Position=11;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe3_pos_11 && counter_position > recipe3_pos_10+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_11();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_75); Displey_speed_75(); task_motor=speed_75; }
     }     
//Позиция 12
    if(counter_position == recipe3_pos_11-1)
     {
      Position=12;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe3_pos_12 && counter_position > recipe3_pos_11+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_12();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_50); Displey_speed_50(); task_motor=speed_50; }
     } 
//Позиция 13
    if(counter_position == recipe3_pos_12-1)
     {
      Position=13;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe3_pos_13 && counter_position > recipe3_pos_12+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_13();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_100); Displey_speed_100(); task_motor=speed_100; }
     }       
 //Позиция 14
    if(counter_position == recipe3_pos_13-1)
     {
      Position=14;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe3_pos_14 && counter_position > recipe3_pos_13+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_14();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_50); Displey_speed_50(); task_motor=speed_50; }
     }  

 //Позиция 14
    if(counter_position == recipe3_pos_14-1)
     {
      Position=15;
      start_time=false;
      privod.privod_stop();
      privod.privod_rele_stop();
     }
     if(counter_position < recipe3_pos_15 && counter_position > recipe3_pos_14+3)
     {
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        Image_15();
      if(bit_AvtoSpeed == false) { privod.privod_task(task_motor); }  if(bit_AvtoSpeed == true) { privod.privod_task(speed_75); Displey_speed_75(); task_motor=speed_75; }
     }                                       
     //
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//END PROGRAM 3
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PROGRAM 4
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void program_4 ()
{ 
        Position=1;
        start_time=true;
        privod.privod_rele_start();
        privod.privod_start();
        privod.privod_task(task_motor);
}
