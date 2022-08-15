// https://github.com/GrantEdwards/uinput-joystick-demo/blob/master/uinput-demo.c
// by @leoneq112 for iNap Malinka, 2022

// SPECIAL SANS VERSION
// allows to fight sans undertale, changed BTN_A and BTN_X to KEY_Z and KEY_X respectively
// also translates left joystick to ARROWS
// have fun! i hope you're filled with   d e t e r m i n a t i o n ❤️ 

#include <stdio.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <bcm2835.h>

#define PIN_BTN_A           1
#define PIN_BTN_B           5
#define PIN_BTN_X           6
#define PIN_BTN_Y           9
#define PIN_BTN_LT          27
#define PIN_BTN_RT          0
#define PIN_BTN_START       16
#define PIN_BTN_SELECT      14
#define PIN_BTN_DPAD_UP     15
#define PIN_BTN_DPAD_DOWN   7
#define PIN_BTN_DPAD_LEFT   22
#define PIN_BTN_DPAD_RIGHT  23
#define PIN_TFT_LED         26

#define CHANNEL_ABS_X       0
#define CHANNEL_ABS_Y       1
#define CHANNEL_ABS_RX      2
#define CHANNEL_ABS_RY      3
#define CHANNEL_BAT         4
#define CHANNEL_BTN         5

#define POLLING_DELAY_US    12000 //still should allow to achieve even 60cps
#define SHUTDOWN_VALUE      270
//#define DEBUG
#define BTN_PULLUP
#define PWM_CHANNEL 0
#define RANGE 1024

#define ABS_X_MIN           150
#define ABS_X_MAX           850
#define ABS_Y_MIN           150
#define ABS_Y_MAX           880
#define ABS_RX_MIN          190
#define ABS_RX_MAX          820
#define ABS_RY_MIN          180
#define ABS_RY_MAX          770

const struct uinput_setup setup =
{
     .name = "Built-in Joystick",
     .id =
     {
      .bustype = BUS_USB,
      .vendor  = 0x3,
      .product = 0x3,
      .version = 2,
     }
};
typedef enum
{
    SLEEP,
    VOL_UP,
    VOL_DOWN,
    MUTE
} SYS_BTN_STATE;
int counter = 0;

uint16_t MCP_readChannel(uint8_t ch);
int UINPUT_initialize();
void UINPUT_setupAbs(int fd, unsigned chan, int min, int max);
int UINPUT_close(int h);
void BCM2835_initialize();
void BCM2835_close();

int main()
{     
    // INITIALIZATION
    printf("malinkabtn-sans: driver startup!\n\r");
    int UINPUT_handle = UINPUT_initialize();
    BCM2835_initialize();

    printf("malinkabtn-sans: running main loop...\n\r");
    // MAIN LOOP
    while(1)
    {
        if(0 == (MCP_readChannel(CHANNEL_BTN) >> 9)) // optimization lvl 100
        {
            SYS_BTN_STATE state = SLEEP;
            while(0 == (MCP_readChannel(CHANNEL_BTN) >> 9))
            {
                if(!bcm2835_gpio_lev(PIN_BTN_LT))
                {
                    state = VOL_DOWN;
                    while(!bcm2835_gpio_lev(PIN_BTN_LT));
                    printf("VOL DOWN\n\r");
                    break;
                }
                else if(!bcm2835_gpio_lev(PIN_BTN_RT))
                {
                    state = VOL_UP;
                    printf("VOL UP\n\r");
                    while(!bcm2835_gpio_lev(PIN_BTN_RT));
                    break;
                }
                else if(!bcm2835_gpio_lev(PIN_BTN_B))
                {
                    state = MUTE;
                    printf("VOL TOGGLE\n\r");
                    while(!bcm2835_gpio_lev(PIN_BTN_B));
                    break;
                }
            }

            switch(state)
            {
                case SLEEP:
                    if(bcm2835_gpio_lev(PIN_TFT_LED) == 1)
                    {
                        bcm2835_gpio_write(PIN_TFT_LED, LOW);
                    }
                    else 
                    {
                        bcm2835_gpio_write(PIN_TFT_LED, HIGH);
                    }
                    break;
                case VOL_UP:
                    system("sudo -u pi amixer set Headphone 265+");
                    break;
                case VOL_DOWN:
                    system("sudo -u pi amixer set Headphone 265-");
                    break;
                case MUTE:
                    system("sudo -u pi amixer set Headphone toggle");
                    break;
            }

        }
        struct input_event ev[17];
        memset(&ev, 0, sizeof(ev));

        ev[0].type = EV_KEY;
        ev[0].code = KEY_Z;
        ev[0].value = !bcm2835_gpio_lev(PIN_BTN_A);

        ev[1].type = EV_KEY;
        ev[1].code = BTN_B;
        ev[1].value = !bcm2835_gpio_lev(PIN_BTN_B);

        ev[2].type = EV_KEY;
        ev[2].code = KEY_X;
        ev[2].value = !bcm2835_gpio_lev(PIN_BTN_X);

        ev[3].type = EV_KEY;
        ev[3].code = BTN_Y;
        ev[3].value = !bcm2835_gpio_lev(PIN_BTN_Y);

        ev[4].type = EV_KEY;
        ev[4].code = BTN_TL;
        ev[4].value = !bcm2835_gpio_lev(PIN_BTN_LT);

        ev[5].type = EV_KEY;
        ev[5].code = BTN_TR;
        ev[5].value = !bcm2835_gpio_lev(PIN_BTN_RT);

        ev[6].type = EV_KEY;
        ev[6].code = BTN_START;
        ev[6].value = !bcm2835_gpio_lev(PIN_BTN_START);

        ev[7].type = EV_KEY;
        ev[7].code = BTN_SELECT;
        ev[7].value = !bcm2835_gpio_lev(PIN_BTN_SELECT);

        ev[8].type = EV_KEY;
        ev[8].code = KEY_W;
        ev[8].value = !bcm2835_gpio_lev(PIN_BTN_DPAD_UP);

        ev[9].type = EV_KEY;
        ev[9].code = KEY_S;
        ev[9].value = !bcm2835_gpio_lev(PIN_BTN_DPAD_DOWN);

        ev[10].type = EV_KEY;
        ev[10].code = KEY_A;
        ev[10].value = !bcm2835_gpio_lev(PIN_BTN_DPAD_LEFT);

        ev[11].type = EV_KEY;
        ev[11].code = KEY_D;
        ev[11].value = !bcm2835_gpio_lev(PIN_BTN_DPAD_RIGHT);

	    int TEMP = MCP_readChannel(CHANNEL_ABS_RX);
	    if(TEMP <= 400) 
        {
            ev[10].type = EV_KEY;
            ev[10].code = KEY_D;
            ev[10].value = 1;
        }
        else 
        {
            ev[10].type = EV_KEY;
            ev[10].code = KEY_D;
            ev[10].value = 0;
        }
        
        if (TEMP >= 600)
        {
            ev[11].type = EV_KEY;
            ev[11].code = KEY_A;
            ev[11].value = 1;
        }
        else 
        {
            ev[11].type = EV_KEY;
            ev[11].code = KEY_A;
            ev[11].value = 0;
        }
        TEMP = MCP_readChannel(CHANNEL_ABS_RY);
        
         if(TEMP <= 400) 
        {
            ev[8].type = EV_KEY;
            ev[8].code = KEY_S;
            ev[8].value = 1;
        }
        else 
        {
            ev[8].type = EV_KEY;
            ev[8].code = KEY_S;
            ev[8].value = 0;
        }
        if (TEMP >= 600)
        {
            ev[9].type = EV_KEY;
            ev[9].code = KEY_W;
            ev[9].value = 1;
        }
        else 
        {
            ev[9].type = EV_KEY;
            ev[9].code = KEY_W;
            ev[9].value = 0;
        }
        

        ev[12].type = EV_ABS;
        ev[12].code = ABS_X;
        ev[12].value = MCP_readChannel(CHANNEL_ABS_X);
        #ifdef DEBUG
            printf("VALUE: %d ", MCP_readChannel(CHANNEL_ABS_X));
            usleep(1000);
        #endif

        ev[13].type = EV_ABS;
        ev[13].code = ABS_Y;
        ev[13].value = MCP_readChannel(CHANNEL_ABS_Y);
        #ifdef DEBUG
            printf("VALUE: %d ", MCP_readChannel(CHANNEL_ABS_Y));
            usleep(1000);
        #endif

        ev[14].type = EV_ABS;
        ev[14].code = ABS_RX;
        ev[14].value = MCP_readChannel(CHANNEL_ABS_RX);
        #ifdef DEBUG
            printf("VALUE: %d ", MCP_readChannel(CHANNEL_ABS_RX));
            usleep(1000);
        #endif

        ev[15].type = EV_ABS;
        ev[15].code = ABS_RY;
        ev[15].value = MCP_readChannel(CHANNEL_ABS_RY);
        #ifdef DEBUG
            printf("VALUE: %d\r\n", MCP_readChannel(CHANNEL_ABS_RY));
            usleep(1000);
        #endif

        ev[16].type = EV_SYN;
        ev[16].code = SYN_REPORT;
        ev[16].value = 0;

        if(write(UINPUT_handle, &ev, sizeof ev) < 0)
        {
            perror("write");
            exit(1);
        }

        if(counter >= 3000)
        {
            uint16_t bat = MCP_readChannel(CHANNEL_BAT);
            FILE* FILE_handle = fopen("battery", "w+");
            fprintf(FILE_handle, "%d", bat);
            fclose(FILE_handle);
            /*
            the pi should shutdown at 3.0v
            */
            if(bat <= SHUTDOWN_VALUE)
            {
                system("sudo shutdown now");
            }
            counter = 0;
        }
        counter++;
        usleep(POLLING_DELAY_US);
    }

    // EXIT
    UINPUT_close(UINPUT_handle);
    BCM2835_close();
    return 0;
}

uint16_t MCP_readChannel(uint8_t ch)
{
    uint8_t tx_buffer[3] = {1, (1<<7) + (ch<<4), 0};
    uint8_t rx_buffer[3];
    bcm2835_aux_spi_transfernb(tx_buffer, rx_buffer, sizeof(tx_buffer));
    uint16_t adc = ((rx_buffer[1] << 8) + rx_buffer[2]);
    //printf("Result: %d\n\r", adc);

    return adc;
}

int UINPUT_initialize()
{
    int inputfd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (inputfd < 0)
    {
        perror("open /dev/uinput");
        exit(1);
    }
    ioctl(inputfd, UI_SET_EVBIT, EV_KEY); // enable button/key handling
    ioctl(inputfd, UI_SET_EVBIT, EV_ABS); // enable analog absolute position handling
  
    ioctl(inputfd, UI_SET_KEYBIT, BTN_A);
    ioctl(inputfd, UI_SET_KEYBIT, BTN_B);
    ioctl(inputfd, UI_SET_KEYBIT, BTN_X);
    ioctl(inputfd, UI_SET_KEYBIT, BTN_Y);
    ioctl(inputfd, UI_SET_KEYBIT, BTN_TL);
    ioctl(inputfd, UI_SET_KEYBIT, BTN_TR);
    ioctl(inputfd, UI_SET_KEYBIT, BTN_START);
    ioctl(inputfd, UI_SET_KEYBIT, BTN_SELECT);
    ioctl(inputfd, UI_SET_KEYBIT, BTN_DPAD_UP);
    ioctl(inputfd, UI_SET_KEYBIT, BTN_DPAD_DOWN);
    ioctl(inputfd, UI_SET_KEYBIT, BTN_DPAD_LEFT);
    ioctl(inputfd, UI_SET_KEYBIT, BTN_DPAD_RIGHT);
    ioctl(inputfd, UI_SET_KEYBIT, KEY_Z);
    ioctl(inputfd, UI_SET_KEYBIT, KEY_UP);
    ioctl(inputfd, UI_SET_KEYBIT, KEY_LEFT);
    ioctl(inputfd, UI_SET_KEYBIT, KEY_DOWN);
    ioctl(inputfd, UI_SET_KEYBIT, KEY_RIGHT);
    ioctl(inputfd, UI_SET_KEYBIT, KEY_X);
    ioctl(inputfd, UI_SET_KEYBIT, KEY_Y);
    ioctl(inputfd, UI_SET_KEYBIT, KEY_W);
	ioctl(inputfd, UI_SET_KEYBIT, KEY_A);
    ioctl(inputfd, UI_SET_KEYBIT, KEY_S);
    ioctl(inputfd, UI_SET_KEYBIT, KEY_D);

    UINPUT_setupAbs(inputfd, ABS_X,  ABS_X_MIN, ABS_X_MAX);
    UINPUT_setupAbs(inputfd, ABS_Y,  ABS_Y_MIN, ABS_Y_MAX);
    UINPUT_setupAbs(inputfd, ABS_RX, ABS_RX_MIN, ABS_RX_MAX);
    UINPUT_setupAbs(inputfd, ABS_RY, ABS_RY_MIN, ABS_RY_MAX);

    if (ioctl(inputfd, UI_DEV_SETUP, &setup))
    {
        perror("UI_DEV_SETUP");
        exit(1);
    }   
  
    if (ioctl(inputfd, UI_DEV_CREATE))
    {
        perror("UI_DEV_CREATE");
        exit(1);
    }

    return inputfd;
}

void UINPUT_setupAbs(int fd, unsigned chan, int min, int max)
{
    if (ioctl(fd, UI_SET_ABSBIT, chan))
        perror("UI_SET_ABSBIT");
  
    struct uinput_abs_setup s =
    {
        .code = chan,
        .absinfo = { .minimum = min,  .maximum = max },
    };
 
    if (ioctl(fd, UI_ABS_SETUP, &s))
        perror("UI_ABS_SETUP");
}

int UINPUT_close(int h)
{
    if(ioctl(h, UI_DEV_DESTROY))
    {
      printf("UI_DEV_DESTROY");
      exit(1);
    }
    close(h);
}

void BCM2835_initialize()
{
    #ifdef DEBUG
        //bcm2835_set_debug(1);
    #endif

    // BCM2835
    if (!bcm2835_init())
    {
        perror("BCM2835_initialize");
        exit(1);
    }


    // AUX SPI
    bcm2835_aux_spi_useChipSelects(0, 1, 0);
    bcm2835_aux_spi_begin();

    if (!bcm2835_aux_spi_begin())
    {
        perror("BCM2835_aux_spi_begin");
        exit(1);
    }

    bcm2835_aux_spi_prepare(BCM2835_AUX_SPI_CS1, 1000000, BCM2835_AUX_SPI_MODE0, BCM2835_AUX_SPI_MSBFIRST, BCM2835_AUX_SPI_DOUTHOLD_0);


    // GPIO
    bcm2835_gpio_fsel(PIN_BTN_A, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(PIN_BTN_B, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(PIN_BTN_X, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(PIN_BTN_Y, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(PIN_BTN_START, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(PIN_BTN_SELECT, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(PIN_BTN_LT, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(PIN_BTN_RT, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(PIN_BTN_DPAD_LEFT, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(PIN_BTN_DPAD_UP, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(PIN_BTN_DPAD_RIGHT, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(PIN_BTN_DPAD_DOWN, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(PIN_TFT_LED, BCM2835_GPIO_FSEL_OUTP);

    #ifdef BTN_PULLUP
        bcm2835_gpio_set_pud(PIN_BTN_A, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_set_pud(PIN_BTN_B, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_set_pud(PIN_BTN_X, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_set_pud(PIN_BTN_Y, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_set_pud(PIN_BTN_START, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_set_pud(PIN_BTN_SELECT, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_set_pud(PIN_BTN_LT, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_set_pud(PIN_BTN_RT, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_set_pud(PIN_BTN_DPAD_LEFT, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_set_pud(PIN_BTN_DPAD_UP, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_set_pud(PIN_BTN_DPAD_RIGHT, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_set_pud(PIN_BTN_DPAD_DOWN, BCM2835_GPIO_PUD_UP);
    #endif

    bcm2835_gpio_write(PIN_TFT_LED, HIGH); //turn on backlight
}

void BCM2835_close()
{
    bcm2835_aux_spi_end();
    bcm2835_close();
}
