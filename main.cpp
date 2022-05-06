/*  LCD Screen Test
*   Designed to test basic functionality of the Nokia N5110 LCD display.    
*   The screen is 84 x 48 pixels with the upper lefthand corner having the
*   designation of (0,0). The x,y co-ordinates of each image and sprite relate
*   to the upper left hand corner of each image.
*
*   Required library: N5110
*   Author: Andrew Knowles 2021. Modified Feb 2022 by Dr Tim Amsdon
*/

#include "mbed.h"
#include "N5110.h"
#include <cstdio>
#include <string>
#define WAIT_TIME_MS 100 
#define MAIN_MENU 0
#define Sche 1
#define LIGHT 1
#define TEMP 2
#define SLEEP 3
#define TIME 4
#define SMARTLIGHT 5
#define NOKEY -1
#define OK 12
#define BACK 10
BufferedSerial pc(USBTX, USBRX, 115200);        //establish serial communications between PC and NUCLEO

AnalogIn LDR(PC_2); //LDR pin
AnalogIn TMP(PC_3); //TEMP Sensor pin
DigitalInOut buttonA(PC_12);
DigitalInOut buttonB(PC_11);
DigitalInOut buttonC(PC_10);
DigitalInOut buttonD(PA_15);
DigitalInOut buttonE(PA_14);
DigitalInOut buttonF(PA_13);
DigitalInOut buttonG(PA_5);
DigitalOut Heating(PB_8);
DigitalOut LightA(PB_9);
int state = 0;
// function prototypes
int keydown = 0;
Ticker ticker;
int h=0;
int m=0;
int s=0;
int ms20=0;
float light_sen = 0;
volatile int g_timer_flag = 0;

void timer_isr();

volatile int key = -1;
int key_row=0;
int key_column=0;
struct schedule{
    int start_h;
    int start_m;
    int end_h;
    int end_m;
};
//Pin assignment format:  lcd(IO, Ser_TX, Ser_RX, MOSI, SCLK, PWM)  
N5110 lcd(PC_7, PA_9, PB_10, PB_5, PB_3, PA_10);
void menu1();
void menu2();
void menu3();
void menu4();
void menu5();
void menu_init();
char T_s[10];
char Input_s[]="  ";
char T_set_s[10];
char Key_s[5];
char Time[10];
char Time_in[10];
int main(){
    //ticker.attach(&timer_isr,1000ms);
    // Button A has a pull-down resistor, so the pin will be at 0 V by default
    // and rise to 3.3 V when pressed. We therefore need to look for a rising edge
    // on the pin to fire the interrupt
    buttonD.output();
    buttonE.output();
    buttonF.output();
    buttonG.output();
    buttonD.mode(PushPullNoPull);
    buttonE.mode(PushPullNoPull);
    buttonF.mode(PushPullNoPull);
    buttonG.mode(PushPullNoPull);
    buttonD=1;
    buttonE=1;
    buttonF=1;
    buttonG=1;
    buttonA.input();
    buttonB.input();
    buttonC.input();
    ticker.attach(&timer_isr,50ms);
    menu_init();
    menu1();
    ThisThread::sleep_for(2s); 
    schedule Schedule[5]={0,0,0,0};
    schedule Schedule_edit={0,0,0,0};
    char Schedule_s[5][10];
    int edit = -1;
    float T_sum=0;
    float T_avg=0;
    int T_read=0;
    int light=0;
    int Input_f=0;
    int T_set=20;
    int edit_num=0;
    int blink=0;
    int smartlight=0;
    int time_select=0;
    int h_in=0;
    int m_in=0;
    
    Schedule[0]={18,0,22,0};
    while(1){
        T_sum=T_sum+TMP.read()*3.3f*100;
        T_read++;
        light_sen=LDR.read();
        if(T_read>5){
        T_avg=T_sum/(float)T_read; 
        T_read=0;
        T_sum=0;
        sprintf(T_s,"T:%.3fC",T_avg);
        }
        //lcd.clear();
        //lcd.printString(T_s,0,2);
        
        //lcd.refresh();
        //ThisThread::sleep_for(1s); 
        //print menu
        switch(state){
            case MAIN_MENU:
            menu1();
            if(key==1){
                state=Sche;
                key = NOKEY;
            }
            else if(key==2){
                state=TEMP;
                key = NOKEY;
            }
            else if(key==3){
                state=SLEEP;
                key=NOKEY;
            }else if(key==4){
                state=TIME;
                key=NOKEY;
            }
            else if(key==5){
                state=SMARTLIGHT;
                key=NOKEY;
            }
            break;
            case Sche:
                lcd.clear();
                for(int i=0;i<5;i++){
                    if(edit == i){
                        blink = !blink;
                        if(blink){
                            sprintf(Schedule_s[i],"%d:%d-%d:%d",Schedule_edit.start_h,Schedule_edit.start_m,Schedule_edit.end_h,Schedule_edit.end_m);
                        }else {
                            switch (edit_num){
                                case 0:
                                if(Schedule_edit.start_h>=10){
                                sprintf(Schedule_s[i],"  :%d-%d:%d",Schedule_edit.start_m,Schedule_edit.end_h,Schedule_edit.end_m);
                                }
                                else{
                                sprintf(Schedule_s[i]," :%d-%d:%d",Schedule_edit.start_m,Schedule_edit.end_h,Schedule_edit.end_m);
                                }
                                break;
                                case 1:
                                if(Schedule_edit.start_m>=10){
                                sprintf(Schedule_s[i],"%d:  -%d:%d",Schedule_edit.start_h,Schedule_edit.end_h,Schedule_edit.end_m);
                                }
                                else{
                                sprintf(Schedule_s[i],"%d: -%d:%d",Schedule_edit.start_h,Schedule_edit.end_h,Schedule_edit.end_m);
                                }
                                break;
                                case 2:
                                if(Schedule_edit.end_h>=10){
                                sprintf(Schedule_s[i],"%d:%d-  :%d",Schedule_edit.start_h,Schedule_edit.start_m,Schedule_edit.end_m);
                                }
                                else{
                                sprintf(Schedule_s[i],"%d:%d- :%d",Schedule_edit.start_h,Schedule_edit.start_m,Schedule_edit.end_m);
                                }
                                break;
                                case 3:
                                if(Schedule_edit.end_m>=10){
                                sprintf(Schedule_s[i],"%d:%d-%d:  ",Schedule_edit.start_h,Schedule_edit.start_m,Schedule_edit.end_h);
                                }
                                else{
                                sprintf(Schedule_s[i],"%d:%d-%d: ",Schedule_edit.start_h,Schedule_edit.start_m,Schedule_edit.end_h);
                                }
                                break;
                            }
                        }
                        lcd.printString(Schedule_s[i], 0, i);
                    }
                    else if(Schedule[i].end_h!=0||Schedule[i].end_m!=0||Schedule[i].start_h!=0||Schedule[i].start_m){
                        sprintf(Schedule_s[i],"%d:%d-%d:%d",Schedule[i].start_h,Schedule[i].start_m,Schedule[i].end_h,Schedule[i].end_m);
                        lcd.printString(Schedule_s[i], 0, i);
                    }
                }
                lcd.refresh();
                if(key>=0 && key<=5 && edit == -1){
                    edit = key;
                    key = NOKEY;
                }else if(edit != -1 && key>=0&&key<=9){
                    switch(edit_num){
                        case 0:
                        if(Schedule_edit.start_h==0){
                            Schedule_edit.start_h=key;
                        }else if(Schedule_edit.start_h<10){
                            Schedule_edit.start_h=Schedule_edit.start_h*10+key;
                        }
                        break;
                        case 1:
                        if(Schedule_edit.start_m==0){
                            Schedule_edit.start_m=key;
                        }else if(Schedule_edit.start_m<10){
                            Schedule_edit.start_m=Schedule_edit.start_m*10+key;
                        }
                        break;
                        case 2:
                        if(Schedule_edit.end_h==0){
                            Schedule_edit.end_h=key;
                        }else if(Schedule_edit.end_h<10){
                            Schedule_edit.end_h=Schedule_edit.end_h*10+key;
                        }
                        break;
                        case 3:
                        if(Schedule_edit.end_m==0){
                            Schedule_edit.end_m=key;
                        }else if(Schedule_edit.end_m<10){
                            Schedule_edit.end_m=Schedule_edit.end_m*10+key;
                        }
                        break;
                    }
                    key = NOKEY;
                }else if(key==BACK){
                    if (edit_num >=0){
                        switch(edit_num){
                        case 0:
                        if(Schedule_edit.start_h==0&&edit_num!=0){
                            edit_num--;
                        }else if(Schedule_edit.start_h<10&&Schedule_edit.start_h>0){
                            Schedule_edit.start_h=0;
                        }else if(Schedule_edit.start_h>=10){
                            Schedule_edit.start_h=Schedule_edit.start_h/10;
                        }else if(Schedule_edit.start_h==0&&edit_num==0){
                        state = MAIN_MENU;
                        edit = -1;
                        }
                        break;
                        case 1:
                        if(Schedule_edit.start_m==0&&edit_num!=0){
                            edit_num--;
                        }else if(Schedule_edit.start_m<10&&Schedule_edit.start_m>0){
                            Schedule_edit.start_m=0;
                        }else if(Schedule_edit.start_m>=10){
                            Schedule_edit.start_m=Schedule_edit.start_m/10;
                        }else if(Schedule_edit.start_m==0&&edit_num==0){
                        state = MAIN_MENU;
                        edit = -1;
                        }
                        break;
                        case 2:
                        if(Schedule_edit.end_h==0&&edit_num!=0){
                            edit_num--;
                        }else if(Schedule_edit.end_h<10&&Schedule_edit.end_h>0){
                            Schedule_edit.end_h=0;
                        }else if(Schedule_edit.end_h>=10){
                            Schedule_edit.end_h=Schedule_edit.end_h/10;
                        }else if(Schedule_edit.end_h==0&&edit_num==0){
                        state = MAIN_MENU;
                        edit = -1;
                        }
                        break;
                        case 3:
                        if(Schedule_edit.end_m==0&&edit_num!=0){
                            edit_num--;
                        }else if(Schedule_edit.end_m<10&&Schedule_edit.end_m>0){
                            Schedule_edit.end_m=0;
                        }else if(Schedule_edit.end_m>=10){
                            Schedule_edit.end_m=Schedule_edit.end_m/10;
                        }else if(Schedule_edit.end_m==0&&edit_num==0){
                        state = MAIN_MENU;
                        edit = -1;
                        }
                        break;
                        }
                    }
                    key = NOKEY;
                }
                else if(key==OK){
                    if (edit_num >=0&&edit_num<3){
                    edit_num++;
                    }else {
                        Schedule[edit]=Schedule_edit;
                        edit=-1;
                        edit_num=0;
                        Schedule_edit={0,0,0,0};
                    }
                    key = NOKEY;
                }
            break;
            case TEMP:
            sprintf(T_set_s,"==>%dC",T_set);
            menu2();
            if(keydown==0&&key!=NOKEY){
                if(key<10&&Input_f<2){
                    Input_s[Input_f]='0'+key;
                    Input_f++;
                }
                else{
                    if(key==OK){
                        T_set=(Input_s[0]-'0')*10+(Input_s[1]-'0');
                        Input_f=0;
                        Input_s[0]=' ';
                        Input_s[1]=' ';
                    }else if(Input_f!=0&&key==BACK){
                        Input_f=0;
                        Input_s[0]=' ';
                        Input_s[1]=' ';
                    }else if(Input_f==0&&key==BACK){
                        Input_f=0;
                        state=MAIN_MENU;
                    }
                }
                key=NOKEY;
            }
            break;
            case SLEEP:
            key=NOKEY;
            lcd.turnOff();
            while(key==NOKEY);
            key=NOKEY;
            menu_init();
            state=MAIN_MENU;
            break;
            case TIME:
            sprintf(Time,"-%02d:%02d-",h,m);
            sprintf(Time_in,"-%02d:%02d-",h_in,m_in);
            if(time_select==0){
                if(key >= 0&&key<=9){
                    if(h_in==0){
                        h_in=key;
                    }
                    else if(h_in>0&&h_in<10){
                        h_in=h_in*10+key;
                    }
                }
                else if(key==OK){
                    time_select++;
                }
            }
            else if(time_select==1){
                if(key >= 0&&key<=9){
                    if(m_in==0){
                        m_in=key;
                        key=NOKEY;
                    }
                    else if(m_in>0&&m_in<10){
                        m_in=m_in*10+key;
                        key=NOKEY;
                    }
                }
                else if(key==OK){
                time_select=0;
                h=h_in;
                m=m_in;
                h_in=0;
                m_in=0;
                }
            }
            key=NOKEY;
            menu3();
            break;
            case SMARTLIGHT:
            if(smartlight==1){
                menu5();
                smartlight=0;
            }else {
                menu4();
                smartlight=1;
            }
            state = MAIN_MENU;
            key=NOKEY;
            break;
            default:
            lcd.printString("   Error   ", 0, 2);
            break; 
        }
        
        //Temp
        if(T_avg<T_set)Heating=1;
        else Heating=0;
        light=0;
        if(smartlight==0){
            for(int i=0;i<5;i++){
                if(Schedule[i].end_h!=0||Schedule[i].end_m!=0||Schedule[i].start_h!=0||Schedule[i].start_m){
                    if((h*60+m)>=(Schedule[i].start_h*60+Schedule[i].start_m)&&(h*60+m)<=(Schedule[i].end_h*60+Schedule[i].end_m))light=1;
                }
            }
            if(light){
                LightA=1;
            }
            else{
                LightA=0;
            }
        }
        else if(smartlight==1){
            light=light_sen>0.4?0:1;
            if(light){
                LightA=1;
            }
            else{
                LightA=0;
            }
        }
        ThisThread::sleep_for(500ms);
    }
}
void menu1(){
        lcd.clear();
        lcd.printString(" -Main Menu- ", 0, 0);
        lcd.printString("1.Schedule", 0, 1); 
        lcd.printString("2.Temperature", 0, 2);
        lcd.printString("3.Sleep", 0, 3);
        lcd.printString("4.Set Time", 0, 4);
        lcd.printString("5.Smart-light", 0, 5);
        sprintf(Key_s,"%d",key);
        if (key>=0) lcd.printString(Key_s,0,4);
        lcd.refresh();
}
void menu_init(){
   lcd.init(LPH7366_1);        //initialise for LPH7366-1 LCD (Options are LPH7366_1 and LPH7366_6)
    lcd.setContrast(0.55);      //set contrast to 55%
    lcd.setBrightness(0.5);     //set brightness to 50% (utilises the PWM)
    lcd.clear();
    lcd.printString(" ============ ", 0, 1);
    lcd.printString("   Starting   ", 0, 2);
    lcd.printString(" ============ ", 0, 3);
    lcd.refresh();
    ThisThread::sleep_for(2s);  
}
void menu2(){
    lcd.clear();
    lcd.printString(T_s, 0, 1);
    lcd.printString(" ============ ", 0, 2);
    lcd.printString(T_set_s, 0, 3);
    lcd.refresh();
}
void menu3(){
    lcd.clear();
    lcd.printString(" ============ ", 0, 0);
    lcd.printString(Time, 0, 1);
    lcd.printString(" ============ ", 0, 2);
    lcd.printString(Time_in, 0, 3);
    lcd.refresh();
}
void menu4(){
    lcd.clear();
    lcd.printString(" ============ ", 0, 1);
    lcd.printString(" Smart light  ", 0, 2);
    lcd.printString("      ON      ", 0, 3);
    lcd.printString(" ============ ", 0, 4);
    lcd.refresh();
}
void menu5(){
    lcd.clear();
    lcd.printString(" ============ ", 0, 1);
    lcd.printString(" Smart light  ", 0, 2);
    lcd.printString("      OFF     ", 0, 3);
    lcd.printString(" ============ ", 0, 4);
    lcd.refresh();
}

void timer_isr(){
    ms20++;
    if(ms20>=20){
        ms20=0;
        s++;
    }
    if(s>=60){
        m++;
        s=0;
    }
    if(m>=60){
        h++;
        m=0;
    }
    if(h>=24){
        h=0;
    }
    if(buttonA.read()||buttonB.read()||buttonC.read()){
        keydown=1;
        if(buttonA.read())key_row = 1;
        else if(buttonB.read())key_row = 2;  
        else if(buttonC.read())key_row = 3; 
        buttonA.output();
        buttonB.output();
        buttonC.output();
        buttonA=1;
        buttonB=1;
        buttonC=1;
        buttonD=0;
        buttonE=0;
        buttonF=0; 
        buttonG=0;
        buttonD.input();
        buttonE.input();
        buttonF.input();
        buttonG.input();
        buttonD.mode(PullNone);
        buttonE.mode(PullNone);
        buttonF.mode(PullNone);
        buttonG.mode(PullNone);
        if(buttonD.read())key_column = 1;
        else if(buttonE.read())key_column = 2;  
        else if(buttonF.read())key_column = 3; 
        else if(buttonG.read())key_column = 4; 
        key = key_row + 3*(key_column-1);
        if(key == 11)key=0;
        buttonD.output();
        buttonE.output();
        buttonF.output();
        buttonG.output();
        buttonD=1;
        buttonE=1;
        buttonF=1;
        buttonG=1;
        buttonA=0;
        buttonB=0;
        buttonC=0;
        buttonA.input();
        buttonB.input();
        buttonC.input();
        buttonA.mode(PullNone);
        buttonB.mode(PullNone);
        buttonC.mode(PullNone);
    }else keydown=0;
}