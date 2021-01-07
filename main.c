//********************************************************************* 
//*                     	FINAL PROJECT                      	
//*                   	McMaster University                     	
//*                  	2DP4 Microcontrollers                    	
//*                      	Lab Section 05                       	     	
//*              	Brooklyn Schmidt schmidb 400065772       	
//*********************************************************************
//*********************************************************************
//*                          Description                             
//*  This program take analog input from an accelerometer and converts                                                         
//*  it to a digital signal using an ADC. It then displays the angle                                                       
//*  onto a breadboard by either using BCD to represent the two digits                             	               
//*  or displaying the information as an increasing bar. The data is
//*  also transmitted to a PC using serial communication and displayed 
//*  on a graph

// filename ******** Main.C ************** 

//***********************************************************************
// Simple ADC example for the Technological Arts EsduinoXtreme board
// by Carl Barnes, 12/03/2014
//***********************************************************************

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"  /* derivative information */
#include "SCI.h"
//#include "math.h"  

char string[20];
unsigned short val; 
unsigned short LSD;
unsigned short MSD;
unsigned short enable = 1;


void setClk(void);
void delay1ms(unsigned int multiple);
void offmode(void);
void mode0(void);
void mode1(void);
//unsigned short arcsin(unsigned short value); 



//---------------------OutCRLF---------------------
// Output a CR,LF to SCI to move cursor to a new line
// Input: none
// Output: none
// Toggle LED each time through the loop

void OutCRLF(void){
  SCI_OutChar(CR);
  SCI_OutChar(LF);
  PTJ ^= 0x20;          // toggle LED D2
}

void main(void) {

  setClk(); 
// Setup and enable ADC channel 0
// Refer to Chapter 14 in S12G Reference Manual for ADC subsystem details

		
	ATDCTL1 = 0x4F;		// set for 12-bit resolution
	ATDCTL3 = 0x88;		// right justified, one sample per sequence
	ATDCTL4 = 0x1F;		// prescaler = 2; ATD clock = 6.25MHz / (2 * (2 + 1)) == 1.04MHz
	ATDCTL5 = 0x26;		// continuous conversion on channel 0
	DDR1AD = 0xBF;        // Set upper 4 bits to outputs (D6, D4, A5, A4 on Esduino) and lower 4 bits to inputs (A3-A0 on Esduino)
  DDRT = 0xFF; 
  DDRM = 0x00;
	
 

// Setup LED and SCI
//ATDDIEN = 0x000F;
  DDRJ |= 0x01;     // PortJ bit 0 is output to LED D2 on DIG13
  SCI_Init(19200);
  
  SCI_OutString("Device Ready:"); OutCRLF();
  for(;;) {
  
    PTJ ^= 0x01;          // toggle LED
    if(PTM == 12 || PTM == 14){   //if the button has been pressed
      delay1ms(30) ;  //debounce checks to see if the button has been pressed 30ms later
      if(PTM == 12 || PTM == 14){   //to make sure that the button pressed was not noise
      
        
      if(enable == 0){ //if "off"
        enable =1;  //if the button is pressed when it is "off" we want to turn it on
        PTT = 0b00010000;  // turns on last LED (b8) to show user it is turning on
        delay1ms(500);
        SCI_OutString("Device Ready"); OutCRLF();  //outputs to show user the device is ready to use again
      }
      else if(enable == 1){
        enable = 0;   //if the button is pressed when it is "on" we want to turn it off
       SCI_OutString("Device on Standby"); OutCRLF(); //displays string to show user that the device is not taking input
       offmode(); //turns off all LEDs
       delay1ms(1000) ;
      }   
    } 
    
    
    }
    
    if(enable == 1){ //if "on"
    
   
    val=ATDDR0;
    
    //linear approximation using 4 different lines in order to approximate arcsin curve
    if(val >= 2000){
     val = 0; //angle is 0 
    } else if(val <= 1625){
     val = 90;
    }  else { 
    if(val>=1710&&val<2000){
    val = 61*(2000-val)/375;
      
    
    }
   
    else if(val <= 1710 && val > 1660){
      val = 66*(2000-val)/375; 
    } else if(val <= 1660 && val > 1640){
      val = 73*(2000-val)/375;
    } else if(val <= 1640 && val > 1630){
      val = 78*(2000-val)/375;
    } else if(val <=1630&&val>1625){
      val = 85*(2000-val)/375;
    }
    }  
    
   
    SCI_OutUDec(val);  //sends value to PC using serial communication
    
    OutCRLF();
    delay1ms(1000);    // 1 sec delay
    
    
    if(PTM == 13){ //checks if the switch is on or off
      mode0();
      } else {
      mode1(); 
      } 
    }
    

  }
}



void delay1ms(unsigned int multiple){
  
  unsigned int i; //loop control variable
  
  TSCR1 = 0x90;   //enable timer and fast timer flag clear
  
  TSCR2 = 0x00;   //Disable timer interrupt, set prescaler=1
  
  TIOS |= 0x01;   //Enable OC0 (not necessary)
  
  TC0 = TCNT + 24000;
   
  for(i=0;i<multiple;i++) {
    TFLG2 = 0x80; //clear the TOF flag
    while (!(TFLG1_C0F));
    TC0 += 6000;
  }
  
  TIOS &= -0x01; //Disable OC0 (not necessary) 
  
}
/////////////////////////////////////////////////////
// NAME: Set Clock
// DESCRIPTION: The following code is adapted from the ESDX User Guide and the Serial Monitor code (S12SerMon2r7) to set the clock speed to 24 MHz. 
// Please reveiw Lecture W8 and Reference Manual for more information on these functions
// NOTE: CLOCK IS ALREADY INCLUDED IN THE MAIN LOOP DO NOT EDIT
/////////////////////////////////////////////////////
#define VCOFRQ 0x00        //VCOFRQ[1:0]  32MHz <= VCOCLK <= 48MHz
#define SYNDIV 0x05        //SYNDIV[5:0]  Syn divide is 11
#define REFFRQ 0x80        //REFFRQ[1:0]  6MHz < fREF <= 12MHz
#define REFDIV 0x01        //REFDIV[3:0]  Ref Divide is 1

void setClk(void){
  CPMUPROT = 0x26;               //Protection of clock configuration is disabled
  
  CPMUCLKS = 0x80;               //PLLSEL=1. Select Bus Clock Source:  PLL clock or Oscillator.
  CPMUOSC = 0x80;                //OSCE=1. Select Clock Reference for PLLclk as:fOSC (8 MHz).
  
  CPMUREFDIV = REFFRQ+REFDIV;    //Set fREF divider and selects reference clock frequency Range. fREF= 4 MHz.  
  
  CPMUSYNR=VCOFRQ + SYNDIV;      //Set Syn divide and selects VCO frequency range. fVCO = 48 MHz.
  
  CPMUPOSTDIV=0x03;              //Set Post Divider (0x00= 0000 0000). fPLL= 48 MHz.
                                 //Overall clock speed is fBUS=fPLL/2=24 MHz  
  
  while (CPMUFLG_LOCK == 0) {}  //Wait for PLL to achieve desired tolerance of target frequency. NOTE: For use when the source clock is PLL. comment out when using external oscillator as source clock 
 
 CPMUPROT = 1;                  //Protection for clock configuration is reenabled 
  //Note: If you change your clock speed you will need to update delay1ms function to give a 1 ms delay
}

void offmode(void){
PT1AD = 0b00000000;
PTT = 0;
}

void mode0(void){
  
   unsigned short temp = val;
   LSD = temp%10;     //change val to temp
   temp = temp-LSD;
   temp = temp/10;
   MSD = temp%10;
   
   val = (MSD & 0b00000011);
   val = (val << 4); 
   LSD = LSD | val;
        PT1AD = LSD;
   val = (MSD & 0b00001100); 
   PTT = val; 
  
}

void mode1(void){

   if(val == 0){
    offmode();
    
   }else if(val < 10){
   
   PT1AD = 0b00000000;
   PTT = 0b00010000; 
   
   }else if(val < 20){
   
   PT1AD = 0b00000000;
   PTT = 0b00011000; 
   
   }else if(val < 30){ 
   
   PT1AD = 0b00000000;
   PTT = 0b00011100; 
   
   }else if(val < 40){
   
   PT1AD = 0b00100000;
   PTT = 0b00011100; 
   
   }else if(val < 50){
   
   PT1AD = 0b00110000;
   PTT = 0b00011100; 
   
   }else if(val < 60){
   
   PT1AD = 0b00111000;
   PTT = 0b00011100; 
   
   }else if(val < 70){
   
   PT1AD = 0b00111100;
   PTT = 0b00011100;  
    
   }else if(val < 80){
   
   PT1AD = 0b00111110;
   PTT = 0b00011100;  
   
   } else {
   
   PT1AD = 0b00111111;
   PTT = 0b00011100; 
  
  
}  
}