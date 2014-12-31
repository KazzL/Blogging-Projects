/*
 * main implementation: use this 'C' sample to create your own application
 *
 */
#include "derivative.h" /* include peripheral declarations */

typedef void(*pt2FuncU8)(uint8_t);    /* Pointer to Functions, UINT8 argument */

#define I2C_MASTER_WRITE            0x00
#define I2C_MASTER_READ             0x01

#define ADT7420_ADDRESS             0x4B
#define ADT7420_TEMPERATURE_REG     0x00
#define ADT7420_STATUS_REG          0x02
#define ADT7420_CONFIG_REG          0x03 
#define ADT7420_T_HIGH_REG          0x04
#define ADT7420_T_LOW_REG           0x06
#define ADT7420_T_CRIT_REG          0x08

//Set ADT7420 control register = 0x43
//[1:0]     Fault queue             - 0b11
//[2]       CT pin Polarity         - 0b0
//[3]       INT pin Polarity        - 0b0    
//[4]       INT/CT mode             - 0b0
//[6:5]     Operation mode          - 0b10
//[7]       Resolution              - 0b0 
#define ADT7420_CONFIG_REG_VAL       0x43

char high_temperature_flag = 0;
char critical_temperature_flag = 0;
char backdoor_open_flag = 0;
char child_present_flag = 0;
char window_open_flag = 0;
char system_error_flag = 0;

/********************************/
//Functions
/********************************/
/**/void gpio_Init();
/*Checked*/void clk_Init();
/**/void uart_Init();
/*Checked*/void i2c_Init();
/*Checked*/void i2c_Data_Trasmit_Delay();
/*Checked*/void i2c_Write(char device_address, char device_register, char data);
/*Checked*/void i2c_Write_Multi(char device_address, char device_register, char* data_array, unsigned char length);
/*Checked*/char i2c_Read(uint8_t device_address, char device_register);
/*Checked*/void i2c_Read_Multi(char device_address, char device_register, char* data_array, unsigned char length);
/*Checked*/char adt7420_Init();
/*Checked*/void kbi_Init();
/*Checked*/void enable_Interrupt(uint8_t vector_number);
/*Checked*/int read_Temperature();
/**/char child_Present();
/*Checked*/void open_Window(int number_of_rotations);
/**/void temperature_Decreasing();
/********************************/
/********************************/




void gpio_Init(){
    // KBI Interrupts, set as Inputs
        // Temperature Interrupt
    GPIOA_PDDR &= ~0x1;                                                 //PTA0 set to input (GPIOA 0)
    GPIOA_PIDR &= ~0x1;                                                 //Enable input
        //Temperature Critical Interrupt
    GPIOA_PDDR &= ~0x2;                                                 //PTA1 set to input (GPIOA 1)
    GPIOA_PIDR &= ~0x2;                                                 //Enable input
        //Backdoor Interrupt
    GPIOA_PDDR &= ~0x4;                                                 //PTA2 set to input (GPIOA 2)
    GPIOA_PIDR &= ~0x4;                                                 //Enable input

    //I2C SDA & SCL lines
    GPIOA_PDIR |= 0xC000;                                               //Disable GPIO and use as I2C 
    
    //LED pins, set as output
    GPIOA_PDDR |= 0x10000;                                             //PTC0 set to output (GPIOA 16)
    GPIOA_PCOR |= 0x10000;                                             //Set output low, turn LED0 off
    GPIOA_PDDR |= 0x20000;                                             //PTC1 set to output (GPIOA 17)
    GPIOA_PCOR |= 0x20000;                                             //Set output low, turn LED1 off
    GPIOA_PDDR |= 0x40000;                                             //PTC2 set to output (GPIOA 18)
    GPIOA_PCOR |= 0x40000;                                             //Set output low, turn LED2 off
    GPIOA_PDDR |= 0x80000;                                             //PTC3 set to output (GPIOA 19)
    GPIOA_PCOR |= 0x80000;                                             //Set output low, turn LED3 off

    //Inputs for child detection
     GPIOA_PDDR &= ~0x1000000;                                           //PTD0 set to input (GPIOA 24) //Bum Sesnor
     GPIOA_PIDR &= ~0x1000000;                                           //Enable input
     GPIOA_PDDR &= ~0x2000000;                                           //PTD1 set to input (GPIOA 25) //Back Sesnor
     GPIOA_PIDR &= ~0x2000000;                                           //Enable input
     GPIOA_PDDR &= ~0x4000000;                                           //PTD2 set to input (GPIOA 26) //Head Sesnor
     GPIOA_PIDR &= ~0x4000000;                                           //Enable input

    //Outputs to stepper motor
    GPIOB_PDDR |= 1 << 16;// 0x10000;                                             //PTG0 set to output (GPIOA 16)
    GPIOB_PCOR |= 1 << 16;// 0x10000;                                             //Set output low
    GPIOB_PDDR |= 1 << 17;// 0x20000;                                             //PTG1 set to output (GPIOA 17)
    GPIOB_PCOR |= 1 << 17;// 0x20000;                                             //Set output low
    GPIOB_PDDR |= 1 << 18;// 0x40000;                                             //PTG2 set to output (GPIOA 18)
    GPIOB_PCOR |= 1 << 18;// 0x40000;                                             //Set output low
    GPIOB_PDDR |= 1 << 19;// 0x80000;                                             //PTG3 set to output (GPIOA 19)
    GPIOB_PCOR |= 1 << 19;// 0x80000; 
}

void clk_Init()
{
    ICS_C1|=ICS_C1_IRCLKEN_MASK;         /* Enable the internal reference clock*/ 
    ICS_C3= 0x50;                        /* Reference clock frequency = 39.0625 KHz*/        
    while(!(ICS_S & ICS_S_LOCK_MASK));   /* Wait for PLL lock, now running at 40 MHz (1024 * 39.0625Khz) */        
    ICS_C2|=ICS_C2_BDIV(1)  ;            /* BDIV=2, Bus clock = 20 MHz*/
    ICS_S |= ICS_S_LOCK_MASK ;           /* Clear Loss of lock sticky bit */
}

void uart_Init()
{
    SIM_SCGC |=  SIM_SCGC_UART2_MASK;
    UART2_BDL= 128;
    UART2_C1 = 0;
    UART2_C2 |= UART_C2_TE_MASK;
    UART2_C2 |= UART_C2_RE_MASK;
    UART2_C2 |= UART_C2_RIE_MASK;
}

void uart_Send_Char(uint8_t send)
{
        while((UART2_S1&UART_S1_TDRE_MASK)==0);
        (void)UART2_S1;
        UART2_D=send;
}

void i2c_Init(){
    SIM_SCGC |= SIM_SCGC_I2C_MASK;
    SIM_PINSEL |= SIM_PINSEL_I2C0PS_MASK;
    I2C0_F = 0x11;    
    I2C0_C1 |= I2C_C1_MST_MASK | I2C_C1_TX_MASK | I2C_C1_IICEN_MASK;
}

void i2c_Data_Trasmit_Delay(){                      //TODO: Add PIT and a lockup bit to prevent lockups and warn of system error
            while ((I2C0_S & I2C_S_IICIF_MASK) == 0){
            //Wait for data to transmit
        }
        I2C0_S |= I2C_S_IICIF_MASK;
}

void i2c_Write(char device_address, char device_register, char data){

    I2C0_C1 &= ~I2C_C1_TXAK_MASK;
    I2C0_C1 |= I2C_C1_MST_MASK | I2C_C1_TX_MASK;                        //Send start bit
    

    I2C0_D = (device_address << 1) | I2C_MASTER_WRITE;                  //Device address with write bit set
    i2c_Data_Trasmit_Delay();

    I2C0_D = device_register;                                           //Device register to write to
    i2c_Data_Trasmit_Delay();

    I2C0_D = data;                                                      //Write to register
    i2c_Data_Trasmit_Delay();

    I2C0_S |= I2C_S_IICIF_MASK;
    I2C0_C1 &= ~I2C_C1_MST_MASK;

    while((I2C0_FLT & I2C_FLT_STOPF_MASK) == 0){                        //Delay for stop bit 
        //Wait for stop bit to transmit
    }
    I2C0_FLT |= I2C_FLT_STOPF_MASK;
}

void i2c_Write_Multi(char device_address, char device_register, char* data_array, unsigned char length){
    int i = 0;

    I2C0_C1 |= I2C_C1_MST_MASK | I2C_C1_TX_MASK | I2C_C1_IICEN_MASK;

    I2C0_C1 &= ~I2C_C1_TXAK_MASK;  
    I2C0_C1 |= I2C_C1_MST_MASK | I2C_C1_TX_MASK;                        //Send start bit
   
    I2C0_D = (device_address << 1) | I2C_MASTER_WRITE;                  //Device address with write bit set
    i2c_Data_Trasmit_Delay();

    I2C0_D = device_register;                                           //Device register to write to
    i2c_Data_Trasmit_Delay();

    for(i = 0; i < length; i++){                                        //Write n bytes
        I2C0_D = data_array[i];
        i2c_Data_Trasmit_Delay();
    }

    I2C0_S |= I2C_S_IICIF_MASK;
    I2C0_C1 &= ~I2C_C1_MST_MASK;

    while((I2C0_FLT & I2C_FLT_STOPF_MASK) == 0){                        //Delay for stop bit 
        //Wait for stop bit to transmit
    }
    I2C0_FLT |= I2C_FLT_STOPF_MASK;
}

char i2c_Read(uint8_t device_address, char device_register){
    char temp;

    I2C0_C1 &= ~I2C_C1_TXAK_MASK;
    I2C0_C1 |= I2C_C1_MST_MASK | I2C_C1_TX_MASK;                        //Send start bit

    I2C0_D = (device_address << 1) | I2C_MASTER_WRITE;                  //Device address with write bit set
    i2c_Data_Trasmit_Delay();

    I2C0_D = device_register;                                           //Device register to read from
    i2c_Data_Trasmit_Delay();

    I2C0_C1 |= I2C_C1_RSTA_MASK;                                        //Send restart
    
    I2C0_D = (device_address << 1) | I2C_MASTER_READ;                   //Device address with read bit set
    i2c_Data_Trasmit_Delay();

    I2C0_C1 &= ~I2C_C1_TX_MASK;                                         //Change to Rx mode

    I2C0_C1 |= I2C_C1_TXAK_MASK;                                        //Set NACK
    temp = I2C0_D;                                                      //Receive data byte
    i2c_Data_Trasmit_Delay();

    I2C0_C1 &= ~I2C_C1_MST_MASK;                                        //Set stop bit
    I2C0_C1 |= I2C_C1_TX_MASK;                                          //Change to Tx mode

    return I2C0_D;
}

void i2c_Read_Multi(char device_address, char device_register, char* data_array, unsigned char length){
    int i = 0;
    char temp;

    I2C0_C1 |= I2C_C1_MST_MASK | I2C_C1_TX_MASK | I2C_C1_IICEN_MASK;

    I2C0_C1 &= ~I2C_C1_TXAK_MASK;
    I2C0_C1 |= I2C_C1_MST_MASK | I2C_C1_TX_MASK;                        //Send start bit

    I2C0_D = (device_address << 1) | I2C_MASTER_WRITE;                  //Device address with write bit set
    i2c_Data_Trasmit_Delay();
    
    I2C0_D = device_register;                                           //Device register to read from
    i2c_Data_Trasmit_Delay();
    
    I2C0_C1 |= I2C_C1_RSTA_MASK;                                        //Send restart
    
    I2C0_D = (device_address << 1) | I2C_MASTER_READ;                   //Device address with read bit set
    i2c_Data_Trasmit_Delay();
    
    I2C0_C1 &= ~I2C_C1_TX_MASK;                                         //Change to Rx mode
    temp = I2C0_D; 
    i2c_Data_Trasmit_Delay();

    for (i = 0; i < length - 2; i ++){                                  //Receive first n - 1 bytes 
        data_array[i] = I2C0_D; 
        i2c_Data_Trasmit_Delay();   
    }

    I2C0_C1 |= I2C_C1_TXAK_MASK;                                        //Receive last byte
    data_array[i] = I2C0_D;
    i2c_Data_Trasmit_Delay();  

    I2C0_C1 &= ~I2C_C1_MST_MASK;                                        //Set stop bit
    I2C0_C1 |= I2C_C1_TX_MASK;                                          //Change to Tx mode
    data_array[i + 1] = I2C0_D;                                         //Read final byte
}

char adt7420_Init(){
    //Setup ADT7420 to operate as requested
    char return_value;
    char return_values[] = {0, 0, 0, 0};
    char set_values[] = {0, 0, 0, 0};

    char t_high0 = 0x19;                                                //35 Celsius
    char t_high1 = 0x00;
    char t_low0 = 0x05;                                                 //10 Celsius
    char t_low1 = 0x00;
    char t_crit0 = 0x20;                                                //65 Celsius
    char t_crit1 = 0x80;

    int attempts = 0;

    while(attempts < 2){
        i2c_Write(ADT7420_ADDRESS, ADT7420_CONFIG_REG, ADT7420_CONFIG_REG_VAL);
        //Read and verify control register
        return_value = i2c_Read(ADT7420_ADDRESS, ADT7420_CONFIG_REG);
        if (return_value == ADT7420_CONFIG_REG_VAL){
            attempts = 0;
            break;                                                      //Carry on
        }
        else{
            attempts++;                                                 //Retry 
        }        
    }

    //Set T High registers
    set_values[0] = t_high0;
    set_values[1] = t_high1;
    while(attempts < 2){
        i2c_Write_Multi(ADT7420_ADDRESS, ADT7420_T_HIGH_REG, set_values, 2);
        //Read and verify control register
        i2c_Read_Multi(ADT7420_ADDRESS, ADT7420_T_HIGH_REG, return_values, 2);
        if (return_values[0] == t_high0 && return_values[1] == t_high1){
            attempts = 0;
            break;                                                      //Carry on
        }
        else{
            attempts++;                                                 //Retry 
        }
    }

    //Set T Low registers
    set_values[0] = t_low0;
    set_values[1] = t_low1;
    while(attempts < 2){
        i2c_Write_Multi(ADT7420_ADDRESS, ADT7420_T_LOW_REG, set_values, 2);
        //Read and verify control register
        i2c_Read_Multi(ADT7420_ADDRESS, ADT7420_T_LOW_REG, return_values, 2);
        if (return_values[0] == t_low0 && return_values[1] == t_low1){
            attempts = 0;
            break;                                                      //Carry on
        }
        else{
            attempts++;                                                 //Retry 
        }
    }

    //Set T Critical registers
    set_values[0] = t_crit0;
    set_values[1] = t_crit1;
    while(attempts < 2){
        i2c_Write_Multi(ADT7420_ADDRESS, ADT7420_T_CRIT_REG, set_values, 2);
        //Read and verify control register
        i2c_Read_Multi(ADT7420_ADDRESS, ADT7420_T_CRIT_REG, return_values, 2);
        if (return_values[0] == t_crit0 && return_values[1] == t_crit1){
            attempts = 0;
            break;                                                      //Carry on
        }
        else{
            attempts++;                                                 //Retry 
        }
    }
    return attempts;
}

void kbi_Init(){                                
    SIM_SCGC |= SIM_SCGC_KBI0_MASK;                                     //Enable clock for KBI module 
    KBI0_SC = 0;
    // Temperature Interrupt
    KBI0_ES &= ~KBI_ES_KBEDG(1);              //TODO: Change back to KBI0                           //Polarity setting, rising edge low level
    KBI0_PE |= KBI_PE_KBIPE(1);              //TODO: Change back to KBI0                           //Enable KBI0 channel 0
    
    // Temperature Critical Interrupt
    // KBI1_ES |= KBI_ES_KBEDG(2);              //TODO: Change back to KBI0                           //Polarity setting, rising edge low level
    // KBI1_PE |= KBI_PE_KBIPE(2);              //TODO: Change back to KBI0                           //Enable KBI0 channel 0

    // backdoor Open Interrupt
    // KBI1_ES &= ~KBI_ES_KBEDG(3);             //TODO: Change back to KBI0                           //Polarity setting, rising edge low level
    // KBI1_PE |= KBI_PE_KBIPE(3);              //TODO: Change back to KBI0                           //Enable KBI0 channel 0
    
    KBI0_SC = 0;                             //TODO: Change back to KBI0                           //Clearing flags
    KBI0_SC |= KBI_SC_KBIE_MASK;             //TODO: Change back to KBI0                           //Enable  KBI0 Interrupts */ 
}

void enable_Interrupt(uint8_t vector_number){
    vector_number = vector_number - 16;
    //Set ICPR & ISER registers accordingly
    NVIC_ICPR |= 1 << (vector_number % 32);
    NVIC_ISER |= 1 << (vector_number % 32);
}

int read_Temperature(){                         //TODO: Consider negitive temps, convert from uint32_t to int16_t
    uint32_t temperature = 0;
    char return_values[] = {0, 0, 0, 0};

    i2c_Write(ADT7420_ADDRESS, ADT7420_CONFIG_REG, ADT7420_CONFIG_REG_VAL);     //Reset the clear data ready bit
    while (((i2c_Read(ADT7420_ADDRESS, ADT7420_STATUS_REG) & 0x80) >> 7) == 1){                    //TODO: Needs a time out to prevent lockups
        asm("nop");                                                     //Wait for data to be ready 
    }

    i2c_Read_Multi(ADT7420_ADDRESS, ADT7420_TEMPERATURE_REG, return_values, 2);
    temperature = (return_values[0] << 8) | return_values[1];
    temperature = temperature >> 7;                                     //Remove flag bits and devide by 16 
    return temperature;
}

char child_Present(){
    //Check to see if inputs resemble child present
    //PTD0(GPIOA 24) = Bum Sensor, PTD1(GPIOA 24) = Back Sensor, PTD2(GPIOA 25) = Head Sensor
   if (((GPIOA_PDIR & 0x07000000) >> 23) >= 3){                         //(PTD0 && (PTD1 || PTD2) != 0) <- two conditions are true
       child_present_flag = 1;
   }
   else{
       child_present_flag = 0;
   }
   return child_present_flag;
}

void open_Window(int number_of_rotations){
    //Set and clear stepper motor ports to open window
    const long int stepper_Sequance[] = {0x10000, 0x30000, 0x20000, 0x60000, 0x40000, 0xC0000, 0x80000, 0x90000};
    const int steps_per_rotation = 4076;
    static int current_step = 0;

    long int delay = 0;
    uint32_t steps_taken = 0;    
    uint32_t current_port_setting;
    
    for (steps_taken = 0; steps_taken < steps_per_rotation * number_of_rotations; steps_taken++){                //One rotation = 4075.7728 steps
        current_port_setting = GPIOB_PDOR;            //TODO: TEST funcinality using port G 4-7                      //Read the current port values
        current_port_setting &= 0xFFF0FFFF;                                 //Clear only the lower four bits
        GPIOB_PDOR = stepper_Sequance[current_step] | current_port_setting; //Set the lower four bits to the desired values
        current_step++;

        //if end of sequence reached move to beginning
        if (current_step > 7){
            current_step = 0;
        }

        //wait for motor to reach new step
        for (delay = 0; delay < 1400; delay++){//1375
            asm("nop");
        }
    }
    window_open_flag = 1;
}

void temperature_Decreasing(){          //TODO: Fill in code
    //Check car temperature is decreasing
    //If not decreasing honk horn & activate flashers
}


int main(void)
{
    uint32_t delay;
    char temp;
    int temperature;
    char i2cReadArray[] = {0,0,0,0,0,0,0,0,0};
    char i2cWriteArray[] = {0,0,0,0,0,0,0,0,0};
    char return_value;

    //System Initilization
    clk_Init();
    gpio_Init();
    uart_Init();
    i2c_Init();
    adt7420_Init();

    kbi_Init();                     /* Initialize KBI module */
    enable_Interrupt(INT_KBI0);     /* Enable KBI0 Interrupts */

    for(delay = 0; delay < 9999; delay++);
    temperature = read_Temperature();
    for(delay = 0; delay < 9999; delay++);
    temperature = read_Temperature();
         
    while(1){
        __asm__("wfi");
//      open_Window(1);
//      while(1) {       
//          for (i = 0; string[i] != '\0'; i++){
//              uart_Send_Char(string[i]);
//      }
    }
}




//Interrupt Handlers
void KBI0_IRQHandler()
{
    if((GPIOA_PDIR & GPIO_PDIR_PDI(0x1)) == 0)                    //Temperature interrupt 
    {
        //Deal with high temperature scenario
        window_open_flag = ~window_open_flag;
        if (!window_open_flag){
          open_Window(1);  
        }
        GPIOA_PTOR |= 0x10000;                                          //Set output low, toggle LED0
    }
    
    // if((GPIOA_PDIR & GPIO_PDIR_PDI(0x2000000))>> 25)                    //Critical temperature interrupt
    // {
    //     //Deal with critical temperature scenario
    //     GPIOA_PTOR |= 0x20000;
    // }  

    // if((GPIOA_PDIR & GPIO_PDIR_PDI(0x4000000))>> 26)                    //Backdoor open interrupt
    // {
    //     //Deal with backdoor open scenario
    //     GPIOA_PTOR |= 0x80000;
    // }    
    GPIOA_PTOR |= 0x40000;                                          //Set output low, toggle LED1
    read_Temperature();//TODO: Should disable interrupts here read in mainand then renable interrupts
    GPIOA_PTOR |= 0x80000;                                          //Set output low, toggle LED2
    KBI0_SC |= KBI_SC_KBACK_MASK;                                       //Clear interrupt flag
}
