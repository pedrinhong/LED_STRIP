//ALUNO:PEDRO NOGUEIRA
//MATRÍCULA: 190134771

#include <msp430.h> 

#define TRUE    1
#define FALSE   0
#define ABERTA  1
#define FECHADA 0
#define NUMERO_DE_LEDS 240
#define FLLN(x) ((x)-1)
#define BBIT0 0xE0   //1110 0000
#define BBIT1 0xF8   //1111 1000
#define BIT24 0x800000

void spi_config(void);
void ta0_config(void);
void adc_config(void);
void DMA_config(void);
void uart_config(void);
void display(int x,int y);
void display_float(unsigned int x);
void inicializacao(void);
void monitora_sw(void);
void delay(long limite);
void display_caractere(char c);
void reset(void);
void iniciar(void);
void gpio_config(void);
void GRB(int led,long green, long red, long blue);
void mclk_20MHz(void);
void usci_a0_config(void);
void transmissao(void);
void inicializacao(void);
void singlecolor(int green, int blue, int red);
void pulse(int greenp, int redp, int bluep, int greenbk, int redbk, int bluebk);
void multiple_pulse(long delay,int greenp1, int redp1, int bluep1,int greenp2, int redp2, int bluep2, int greenbk, int redbk, int bluebk);
void spot(char both, char xy,int x, int y, char green1, char red1, char blue1, char green2, char red2, char blue2, char greenbk, char redbk, char bluebk);
void trail(int c, char greenr, char redr, char bluer,char greenl, char redl, char bluel ,char greenbk, char redbk, char bluebk);
void claps(void);

volatile int x,y;
volatile int cont=0,flag=FALSE,i=0,time=1,steps=1,cont1=1,xmax=0;
volatile char modo='1';
char data[NUMERO_DE_LEDS][24];
int j=0,aux=0,diviser;

int main(void){
    //Os " * " nao começam na linha central porque as conversoes do JoyStick na posiçao de repouso nao sao acuradamente o valor mediano, que seria 4095/2->2047 = 0x7FF,
    // Eles sao na verdade: 2002 = 0x72D para o eixo X  e  2083 = 0x823 para o eixo Y. **PARA O MEU JOYSTICK**

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    spi_config();
    ta0_config();
    adc_config();
    uart_config();
    inicializacao();
    mclk_20MHz();
    gpio_config();
    __enable_interrupt();

    diviser=(4095/NUMERO_DE_LEDS);

    while(1){
        if(flag==TRUE){
            flag=FALSE;

            switch(modo){
            case '1': P2IE=0; ADC12IE = ADC12IE15; spot('n', 'x', x, y, 0, 0, 255, 0, 0, 0, 0, 0, 0);    break;
            case '2': P2IE=0; ADC12IE = ADC12IE15; spot('n', 'y', x, y, 0, 255, 255, 0, 0, 0, 100, 0, 0);  break;
            case '3': P2IE=0; ADC12IE = ADC12IE15; spot('y', ' ', x, y, 0, 0, 255 ,0 , 255, 0, 0, 0, 0); break;
            case '4': P2IE=0; ADC12IE = ADC12IE15; trail(x, 0, 0, 255, 0, 255, 0, 0, 0, 0);              break;
            case '5': P2IE=0; ADC12IE = ADC12IE15; trail(y, 255, 0, 255, 0, 120, 189, 0, 255, 0);          break;
            case '6': P2IE=1; ADC12IE = 0 ;       claps();                                     break;
            }

        }
    }
    return 0;
}

#pragma vector = ADC12_VECTOR
void interrupt ADC_INT(void){ //Interrupçao para armazenamento da conversao.
    ADC12IFG=0;
    if((modo=='1')||(modo=='3')||(modo=='4')){
    x=ADC12MEM0+ADC12MEM2+ADC12MEM4+ADC12MEM6+ADC12MEM8+ADC12MEM10+ADC12MEM12+ADC12MEM14;                //Ler resultado
    x=x>>3;
    }
    if((modo=='2')||(modo=='3')||(modo=='5')){
        y=ADC12MEM1+ADC12MEM3+ADC12MEM5+ADC12MEM7+ADC12MEM9+ADC12MEM11+ADC12MEM13+ADC12MEM15;                //Ler resultado
        y=y>>3;
    }
    flag=TRUE;
}

#pragma vector = 42
void interrupt P2_INT(void){ //Interrupçao para armazenamento da conversao.
    delay(100000);
    P2IV;
    cont1++;
    if(cont1==11) cont1=1;
}

#pragma vector = 46
void interrupt BLUETOOTH_INT(void){ //Interrupçao para armazenamento da conversao.
    if(((UCA1RXBUF=='1')||(UCA1RXBUF=='2')||(UCA1RXBUF=='3')||(UCA1RXBUF=='4')||(UCA1RXBUF=='5')||(UCA1RXBUF=='6'))) modo=UCA1RXBUF;
}

// Just a spot
void spot(char both, char xy,int x, int y, char green1, char red1, char blue1, char green2, char red2, char blue2, char greenbk, char redbk, char bluebk){ // Both X and Y? Yes('y') or No('n'), if No, X('x') or Y('y').
    volatile int on, xon, yon;
    xon=(x/diviser);
    yon=(y/diviser);

    if(both=='n'){

        if(xy=='x'){
            on=xon;
        }else on=yon;

        for(aux=1;aux<=NUMERO_DE_LEDS;aux++){
            if(aux==(NUMERO_DE_LEDS-on)){
                GRB(aux,green1,red1,blue1);
            }else GRB(aux,greenbk,redbk,bluebk);
        }
    }

    else{
        for(aux=1;aux<=NUMERO_DE_LEDS;aux++){
            if(aux==(NUMERO_DE_LEDS-xon)){
                GRB(aux,green1,red1,blue1);
            }
            else if(aux==(NUMERO_DE_LEDS-yon)){
                GRB(aux,green2,red2,blue2);
            }
            else GRB(aux,greenbk,redbk,bluebk);
        }
    }

    transmissao();
    reset();
}

// The spot followed by its trail.
void trail(int c, char greenr, char redr, char bluer,char greenl, char redl, char bluel ,char greenbk, char redbk, char bluebk ){
    // R means right and L means left (directions).
    volatile int half,on;

    half=(NUMERO_DE_LEDS/2);
    on=(c/diviser);

    for(aux=1;aux<=NUMERO_DE_LEDS;aux++){

        if(on>half){
            if((aux<half) && (aux>(NUMERO_DE_LEDS-on))){
                GRB(aux,greenr,redr,bluer);}
            else GRB(aux,greenbk,redbk,bluebk);
        }

        else{
            if((aux>half) && (aux<(NUMERO_DE_LEDS-on))){
                GRB(aux,greenl,redl,bluel);}
            else GRB(aux,greenbk,redbk,bluebk);
        }
    }

    transmissao();
    reset();
}

void claps(void){

        // Arbitrary values to the RGB colors
        switch(cont1){
        case 1: singlecolor(255,0,0)                              ; break;
        case 2: singlecolor(0,255,0)                              ; break;
        case 3: singlecolor(0,0,255)                              ; break;
        case 4: pulse(252,124,0,0,0,0)                            ; break;
        case 5: pulse(0,128,128,0,0,0)                            ; break;
        case 6: pulse(0,0,0,191,0,255)                            ; break;
        case 7: multiple_pulse(0,0,255,0,0,0,255,0,0,0)           ; break;
        case 8: multiple_pulse(0,127,255,0,255, 0, 255,0,0,0)     ; break;
        case 9: multiple_pulse(0,255,255,0,0,255,0,100,100,100)   ; break;
        case 10: singlecolor(0,0,0)                               ; break;
        }

}

void pulse(int greenp, int redp, int bluep, int greenbk, int redbk, int bluebk){
    // P refers to the pulse color
    // BK refers to the background color
    for(aux=1;aux<NUMERO_DE_LEDS;aux++){
        GRB(aux,greenp,redp,bluep);
        GRB(aux-1,greenbk,redbk,bluebk);
        transmissao();
    }
    for(aux=NUMERO_DE_LEDS;aux>=1;aux--){
        GRB(aux,greenbk,redbk,bluebk);
        GRB(aux-1,greenp,redp,bluep);
        transmissao();
    }
}

void multiple_pulse(long delay,int greenp1, int redp1, int bluep1,int greenp2, int redp2, int bluep2, int greenbk, int redbk, int bluebk){
    // P refers to the pulse color
    // BK refers to the background color
    for(aux=1;aux<=NUMERO_DE_LEDS;aux++){
        //GRB(aux+1,0,0,0);
        GRB(aux,greenp1,redp1,bluep1);
        GRB(aux-1,greenbk,redbk,bluebk);
        GRB(NUMERO_DE_LEDS-aux+1,greenp2,redp2,bluep2);
        GRB(NUMERO_DE_LEDS-aux+2,greenbk,redbk,bluebk);
        transmissao();
        //reset();
        //atraso(delay);
    }
    for(aux=NUMERO_DE_LEDS;aux>=1;aux--){
        GRB(aux-1,greenp1,redp1,bluep1);
        GRB(aux,greenbk,redbk,bluebk);
        GRB(NUMERO_DE_LEDS-aux+1,greenp2,redp2,bluep2);
        GRB(NUMERO_DE_LEDS-aux,greenbk,redbk,bluebk);
        transmissao();
        //reset();
        //atraso(delay);
    }
}

void singlecolor(int green, int red, int blue){
    for(aux=1;aux<=NUMERO_DE_LEDS;aux++){
        GRB(aux,green,red,blue);
    }
    transmissao();
}

void uart_config(void){
    // configurando  USCI_Ax(0 ou 1)
    UCA1CTL1=UCSWRST; // primeiro passo é sempre setar o reset e ,logo sem seguida, configurar o controle 0
    UCA1CTL0=0;
    // Configurando o Baud-Rate 9.600
    UCA1BRW = 130;                          //Divisor
    UCA1MCTL=UCBRF_3 | UCBRS_0 |UCOS16; //Moduladores no modo de Super Amostragem, BRF(16bits) e BRS(8bits)
    UCA1CTL1=UCSSEL_2;                    //Using ACLK because SMCLOCK was modified

    UCA1IE=UCRXIE;

    P4SEL |= BIT1 | BIT2; //Disponibilizar P4.i e P4.j
    PMAPKEYID = 0X02D52; //Liberar mapeamento de P4
    P4MAP2 = PM_UCA1TXD; //P4.i = TXD
    P4MAP1 = PM_UCA1RXD; //P4.j = RXD
}

void spi_config(void){

    UCA0CTL1 = UCSSEL_2 | UCSWRST;  //SMCLK e RST=1
    UCA0CTL0 = UCMSB | UCMST | UCSYNC;      //Mestre
    UCA0BRW = 3;            // 6,66 MHz Aprox.
    P3SEL |= BIT4 | BIT3;   //Disponibilizar P3.3(SIMO) e P3.4(SOMI)
    P2SEL |= BIT7;          //Disponibilizar P2.7 (CLK)
    UCA0CTL1 = UCSSEL_2;    //RST=0 e Selecionar SMCLK

}

void ta0_config(void){

    TA0CTL  = TASSEL_2 | MC_1 ;//|ID_3;
    //TA0EX0= TAIDEX_7;
    TA0CCTL1 = OUTMOD_6;
    TA0CCR0 = 200;             //16 Hz ==> 16 conversoes por segundo
    TA0CCR1 = TA0CCR0/2;    //carga=50%
    P1DIR |= BIT2;
    P1SEL |= BIT2;          //TA0.1 = P1.2

}

void adc_config(void){
    ADC12CTL0  &= ~ADC12ENC;        //Desabilitar para configurar
    ADC12CTL0 = ADC12ON;           //Ligar ADC
                //ADC12ENC;           //Hab ADC
    ADC12CTL1 = ADC12CSTARTADD_0 |  //Start=0
                ADC12SHS_1       |  //Disp com TA0.1
                ADC12DIV_0       |  //Div=0 clock do ADC
                ADC12SSEL_3      |  //SMCLK
                ADC12CONSEQ_3;      //Seq canais com repetição
    ADC12CTL2 = ADC12RES_2;         //12 bits

    ADC12MCTL0   = ADC12INCH_2 | ADC12SREF_0;  //Entrada A0 (P6.2)
    ADC12MCTL1   = ADC12INCH_3 | ADC12SREF_0;  //Entrada A1 (P6.3)
    ADC12MCTL2   = ADC12INCH_2 | ADC12SREF_0;  //Entrada A0 (P6.2)
    ADC12MCTL3   = ADC12INCH_3 | ADC12SREF_0;  //Entrada A1 (P6.3)
    ADC12MCTL4   = ADC12INCH_2 | ADC12SREF_0;  //Entrada A0 (P6.2)
    ADC12MCTL5   = ADC12INCH_3 | ADC12SREF_0;  //Entrada A1 (P6.3)
    ADC12MCTL6   = ADC12INCH_2 | ADC12SREF_0;  //Entrada A0 (P6.2)
    ADC12MCTL7   = ADC12INCH_3 | ADC12SREF_0;  //Entrada A1 (P6.3)
    ADC12MCTL8   = ADC12INCH_2 | ADC12SREF_0;  //Entrada A0 (P6.2)
    ADC12MCTL9   = ADC12INCH_3 | ADC12SREF_0;  //Entrada A1 (P6.3)
    ADC12MCTL10  = ADC12INCH_2 | ADC12SREF_0;  //Entrada A0 (P6.2)
    ADC12MCTL11  = ADC12INCH_3 | ADC12SREF_0;  //Entrada A1 (P6.3)
    ADC12MCTL12  = ADC12INCH_2 | ADC12SREF_0;  //Entrada A0 (P6.2)
    ADC12MCTL13  = ADC12INCH_3 | ADC12SREF_0;  //Entrada A1 (P6.3)
    ADC12MCTL14  = ADC12INCH_2 | ADC12SREF_0;  //Entrada A0 (P6.2)
    ADC12MCTL15  = ADC12INCH_3 | ADC12SREF_0 |ADC12EOS;  //Entrada A1 (P6.3)

    ADC12CTL0  |= ADC12ENC;        //Desabilitar para configurar
    P6DIR &= ~BIT2;
    //P6DIR &= ~BIT3;
    P6SEL |= BIT2;
    //P6DIR &= ~BIT4; //S1 = P2.1 = entrada
    //P6REN |= BIT4; //Habilitar resistor
    //P6OUT |= BIT4; //Habilitar pullup
    ADC12IE = ADC12IE15;     //ADC12IFG15 Interrompe
}

void gpio_config(void){

    P1DIR|=BIT4|BIT3;
    P1SEL|=BIT4;
    P1OUT &= ~BIT3;

    P2DIR &= ~BIT0; //S1 = P2.1 = entrada
    P2REN |= BIT0; //Habilitar resistor
    P2OUT |= BIT0; //Habilitar pullup

    P2IE=1;
    P2IES=1;
}

void delay(long limite){
    volatile long cont=0;
    while (cont++ < limite) ;
}

void mclk_20MHz(void){

    // Configure crystal ports
    P5SEL |= BIT2 | BIT3 | BIT4 | BIT5; // Configure P5 to use Crystals

    // This should make XT1 startup in 500ms and XT2 in less than 1ms
    UCSCTL6 = XT2DRIVE_3 | // Turn up crystal drive to
            XT1DRIVE_3 | // speed up oscillator startup
            XCAP_3 | // Use maximum capacitance (12pF)
            // XT1OFF | // Make sure XT1 and XT2
            // XT2OFF | // oscillators are active
            // SMCLKOFF | // Leave SMCLK ON
            // XT1BYPASS | // Not using external clock source
            // XT2BYPASS | // Not using external clock source
            // XTS | // XT1 low-frequency mode, which
            0; // means XCAP bits will be used

    UCSCTL0 = 0x00; // Let FLL manage DCO and MOD

    UCSCTL1 = DCORSEL_6 | // Select DCO range to around 20MHz
            // DISMOD | // Enable modulator
            0;

    UCSCTL2 = FLLD_0 | // (D=1) Set FLL dividers
            FLLN(5); // DCOCLK = 1 * 5 * FLLREF = 20 MHz

    UCSCTL3 = SELREF_5 | // Use XT2 (4 MHz) Oscillator for FLLREF
            FLLREFDIV_0; // divided by 1

    UCSCTL5 = DIVPA_0 | // Output dividers to 1
            DIVA_0 | // ACLK divided by 1
            DIVS_0 | // SMCLK divided by 4
            DIVM_0; // MCLK divided by 1

    UCSCTL7 = 0; // Clear XT2,XT1,DCO fault flags

    UCSCTL8 = SMCLKREQEN | // Enable conditional requests for
            MCLKREQEN | // SMCLK, MCLK and ACLK. In case one
            ACLKREQEN; // fails, another takes over

    do { // Check if all clocks are oscillating
        UCSCTL7 &= ~( XT2OFFG | // Try to clear XT2,XT1,DCO fault flags,
                XT1LFOFFG | // system fault flags and check if
                DCOFFG ); // oscillators are still faulty
        SFRIFG1 &= ~OFIFG; //
    } while (SFRIFG1 & OFIFG); // Exit only when everything is ok

    UCSCTL6 &= ~(XT1DRIVE_3 | // Xtal is now stable,
            XT2DRIVE_3); // reduce drive strength (to save power)

    UCSCTL4 = SELA__XT1CLK | // ACLK = XT1 => 32.768 Hz
            SELS__DCOCLK | // SMCLK = DCO => 20.000.000 Hz
            SELM__DCOCLK; // MCLK = DCO => 20.000.000 Hz
}

void GRB(int led,long green, long red, long blue){ // function to turn the decimal value into binary
    volatile long int vector=0,aux=0;
    if((led>NUMERO_DE_LEDS)||(led<1)) return;
    led-=1;
    vector = (green<<16)|(red<<8)|blue;
    aux=BIT24;
    for(i=0;i<24;i++){
        aux=(vector&BIT24)>>23;
        vector=vector<<1;
        if(aux==0) data[led][i]=BBIT0;
        else data[led][i]=BBIT1;
    }
}

void transmissao(void){
    ADC12IE=0;
    for(j=0; j<NUMERO_DE_LEDS; j++){
        for(i=0; i<24; i++){                //24 x 1
            while ( (UCA0IFG&UCTXIFG)==0);  //Esperar TXIFG=1
            UCA0TXBUF=data[j][i];
        }
    }
    ADC12IE = ADC12IE15;
}

void inicializacao(void){
    for(j=0; j<NUMERO_DE_LEDS; j++){
        for(i=0; i<24; i++){                //24 x 1
            data[j][i]=BBIT0;
        }
    }
}
void reset(void){
    int i;
    for (i=0; i<50; i++){
        while ( (UCA0IFG&UCTXIFG)==0);  //Esperar TXIFG=1
        UCA0TXBUF=0;
    }

}

void iniciar(void){
    int i;
    if(cont==0){
        cont=1;
        for (i=0; i<50; i++){
            while ( (UCA0IFG&UCTXIFG)==0);  //Esperar TXIFG=1
            UCA0TXBUF=0xFF;
        }
    }
}
