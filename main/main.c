#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "asm330lhh_reg.h"
#include "asm330lhh_cda.h"
#include "asm330lhh_cda.c"

// #include "nvs.h" //non volatile storage important for saving data while code runs
// #include "nvs_flash.h" // storing in nvs flash memory
// #include "esp_bt.h" //bluetooth related data structs and important files init cb etc
// #include "esp_bt_main.h" // initialize the Bluetooth stack, enable Bluetooth functionality, and retrieve the device address on ESP32
// #include "esp_gap_bt_api.h" //is used to set the scan mode for the device after the Bluetooth stack is initialized and enabled
// #include "esp_bt_device.h" //device address and device name for bluetooth
// #include "esp_spp_api.h" // are used to manage SPP connections, perform service discovery, and handle data transmission over SPP.
// #include "time.h" //time funtions
// #include "sys/time.h"//time funtions

#define WHO_AM_I_REG 0x0F
#define REG_CNTRL1_XL 0x10
#define REG_OUTX_L_G 0x22
#define REG_OUTX_H_G 0x23
#define REG_OUTY_L_G 0x24
#define REG_OUTY_H_G 0x25
#define REG_OUTZ_L_G 0x26
#define REG_OUTZ_H_G 0x27
#define REG_OUTX_L_A 0x28
#define REG_OUTX_H_A 0x29
#define REG_OUTY_L_A 0x2A
#define REG_OUTY_H_A 0x2B
#define REG_OUTZ_L_A 0x2C
#define REG_OUTZ_H_A 0x2D
#define SPI_READ_MASK 0x80 // MSB = 1 for read operation

#define CLOCK_SPEED 3333

#define SENSOR_BUS SPI3_HOST

#define BOOT_TIME 1000

// #define DEVICE_NAME "Ahan's ESP BT"
// #define SPP_SERVER_NAME "Ahan's SPP Server"

// static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
// static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
// static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

static const char TAG[] = "VSPI";
spi_device_handle_t spi;

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static void platform_delay(uint32_t ms);
static void platform_init(void);

// static void esp_spp_cb(esp_spp_cb_event_t event,esp_spp_cb_param_t *param){

//     int num_connections = 0;

//     switch (event){
//         case ESP_SPP_INIT_EVT:
//             if(param->init.status == ESP_SPP_SUCCESS){
//             ESP_LOGI(TAG,"ESP_SPP_INIT_EVNT"); //log value shows this event
//             esp_spp_start_srv(sec_mask, role_slave, 0, SPP_SERVER_NAME); /*for starting we declare
//             security method and role of esp32 and its server name*/}
//             else {
//                 ESP_LOGE(TAG, "ESP_SPP_INIT_EVT status:%d", param->init.status);
//             }//incase of failure it prints status value
//             break;
//         case ESP_SPP_DISCOVERY_COMP_EVT:
//             ESP_LOGI(TAG, "ESP_SPP_DISCOVERY_COMP_EVT"); //discovery event occuring
//             break;
//         case ESP_SPP_OPEN_EVT:
//             ESP_LOGI(TAG, "ESP_SPP_OPEN_EVT"); //event is open for connections
//             if (num_connections < 2) {
//                 num_connections++;
//             } else {
//                 printf("Max connections reached. Rejecting new connection.\n");
//                 esp_spp_disconnect(param->open.handle);
//             }
//             break;
//         case ESP_SPP_CLOSE_EVT:
//                 printf("ESP_SPP_CLOSE_EVT status:%d handle:%lu close_by_remote:%d\n", param->close.status, param->close.handle, param->close.async);
//                 break;
//         case ESP_SPP_START_EVT: // esp spp event when starts
//                 if (param->start.status == ESP_SPP_SUCCESS) {
//                     printf("ESP_SPP_START_EVT handle:%lu sec_id:%d scn:%d\n", param->start.handle, param->start.sec_id, param->start.scn);
//                     esp_bt_gap_set_device_name(DEVICE_NAME);
//                     esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
//                 } else {
//                     ESP_LOGE(TAG, "ESP_SPP_START_EVT status:%d", param->start.status);
//                 }
//                 break;
//         case ESP_SPP_CL_INIT_EVT:
//                 ESP_LOGI(TAG, "ESP_SPP_CL_INIT_EVT");//when the SPP client is initialized,
//                 break;
//         case ESP_SPP_DATA_IND_EVT: //if event data print value received
//                 printf("Received data:\t %.*s", param->data_ind.len, param->data_ind.data);
//             break;
//             case ESP_SPP_WRITE_EVT:
//                 // ESP_LOGI(TAG, "ESP_SPP_WRITE_EVT"); // when we write from esp32 to mobile phone
//                 uint8_t text[10];
//                 scanf("To server: %s", text);
//                 esp_spp_write(param->open.handle, 10, text);
//                 break;
//             default:
//                 break;    
//     }
// }

void app_main(void)
{

    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_0, 0);

    stmdev_ctx_t dev_ctx;
    dev_ctx.write_reg = platform_write;
    dev_ctx.read_reg = platform_read;
    dev_ctx.mdelay = platform_delay;
    dev_ctx.handle = &spi;

    uint8_t whoamI, rst;
    // uint8_t arr[2] = {0xFF, 0xFF};

    platform_init();

    platform_delay(BOOT_TIME);

    while(1) {
        asm330lhh_device_id_get(&dev_ctx, &whoamI);
        ESP_LOGI(TAG, "WHOAMI : 0x%02X", whoamI);
        if (whoamI == ASM330LHH_ID) break;
        platform_delay(1000);
    }

    /* Restore default configuration */
    asm330lhh_reset_set(&dev_ctx, PROPERTY_ENABLE);

    do {
        asm330lhh_reset_get(&dev_ctx, &rst);
    } while (rst);
 

    asm330lhh_device_conf_set(&dev_ctx, PROPERTY_ENABLE);
    // asm330lhh_spi_mode_set(&dev_ctx, 0);

    asm330lhh_xl_data_rate_set(&dev_ctx, ASM330LHH_XL_ODR_3333Hz);
    // asm330lhh_xl_full_scale_set(&dev_ctx, ASM330LHH_2g);

    // asm330lhh_gy_data_rate_set(&dev_ctx, ASM330LHH_GY_ODR_3333Hz);
    // asm330lhh_gy_full_scale_set(&dev_ctx, ASM330LHH_2000dps);

    // asm330lhh_auto_increment_set(&dev_ctx, 1);

    dev_ctx.read_reg(dev_ctx.handle, WHO_AM_I_REG, &whoamI, 1);
    ESP_LOGI(TAG, "WHO AM I: 0x%02X", whoamI);

    uint8_t ctrl4;
    dev_ctx.read_reg(dev_ctx.handle, 0x10, &ctrl4, 1);
    ESP_LOGI(TAG, "CTRL1: 0x%02X", ctrl4);

    

    // static esp_spp_cfg_t spp_cfg;
    // memset(&spp_cfg, 0 ,sizeof(spp_cfg));
    // spp_cfg.mode = ESP_SPP_MODE_CB;
    // spp_cfg.enable_l2cap_ertm = true;

    // nvs_flash_init(); //it will save important details necesasry for run time program
    // esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT(); //bt config default
    // esp_bt_controller_init(&bt_cfg); //init this default configuration
    // esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT); //enabling classic bluetooth mode
    // esp_bluedroid_init(); //initialize bluedroid
    // esp_bluedroid_enable(); //enable bluedroid
    // esp_spp_register_callback(esp_spp_cb); //event handler registerd here
    // esp_spp_init(ESP_SPP_MODE_CB); // initialize the esp_spp_mode call back explained above

    int16_t offset[3] = {0,0,0};
    // asm330lhh_acceleration_raw_get(&dev_ctx, &val);

    platform_delay(100);

    uint8_t buff[6];
    memset(buff, 0, sizeof(uint8_t)*6);
    dev_ctx.read_reg(dev_ctx.handle, REG_OUTX_L_A, buff, 1);
    dev_ctx.read_reg(dev_ctx.handle, REG_OUTX_H_A, buff+1, 1);
    dev_ctx.read_reg(dev_ctx.handle, REG_OUTY_L_A, buff+2, 1);
    dev_ctx.read_reg(dev_ctx.handle, REG_OUTY_H_A, buff+3, 1);
    dev_ctx.read_reg(dev_ctx.handle, REG_OUTZ_L_A, buff+4, 1);
    dev_ctx.read_reg(dev_ctx.handle, REG_OUTZ_H_A, buff+5, 1);

    offset[0] = (int16_t)buff[1];
    offset[0] = (offset[0] * 256) + (int16_t)buff[0];
    offset[1] = (int16_t)buff[3];
    offset[1] = (offset[1] * 256) + (int16_t)buff[2];
    offset[2] = (int16_t)buff[5];
    offset[2] = (offset[2] * 256) + (int16_t)buff[4];

    while(true){

        int16_t val[3] = {0,0,0};
        // asm330lhh_acceleration_raw_get(&dev_ctx, &val);
        asm330lhh_acceleration_get(&dev_ctx, &val[0]);

        val[0] -= offset[0];
        val[1] -= offset[1];
        val[2] -= offset[2];

        // memset(buff, 0, sizeof(uint8_t)*6);
        // dev_ctx.read_reg(dev_ctx.handle, REG_OUTX_L_A, buff, 1);
        // dev_ctx.read_reg(dev_ctx.handle, REG_OUTX_H_A, buff+1, 1);
        // dev_ctx.read_reg(dev_ctx.handle, REG_OUTY_L_A, buff+2, 1);
        // dev_ctx.read_reg(dev_ctx.handle, REG_OUTY_H_A, buff+3, 1);
        // dev_ctx.read_reg(dev_ctx.handle, REG_OUTZ_L_A, buff+4, 1);
        // dev_ctx.read_reg(dev_ctx.handle, REG_OUTZ_H_A, buff+5, 1);

        // val[0] = (int16_t)buff[1];
        // val[0] = (val[0] * 256) + (int16_t)buff[0];
        // val[1] = (int16_t)buff[3];
        // val[1] = (val[1] * 256) + (int16_t)buff[2];
        // val[2] = (int16_t)buff[5];
        // val[2] = (val[2] * 256) + (int16_t)buff[4];

        if(sum_abs_diff(&val) == 1) {
            ESP_LOGI(TAG, "Crash Detected!\n");
            gpio_set_level(GPIO_NUM_0, 1);
        }

        // printf("%0.2f,%0.2f,%0.2f\n", (val[0] - offset[0])*0.061, (val[1] - offset[1])*0.061, (val[2] - offset[2])*0.061);
        // ESP_LOGI(TAG,"X: %0.2f Y: %0.2f Z: %0.2f", (val[0] - offset[0])*0.061, (val[1] - offset[1])*0.061, (val[2] - offset[2])*0.061);

        vTaskDelay(1);
    }
}

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len){

    spi_transaction_t write_trans;
    memset(&write_trans, 0, sizeof(write_trans));
    uint16_t write_buffer;
    int num_bits = 8 + 8*len;
    
    // uint8_t full_command[2] = {*bufp, reg};
    uint16_t full_command = (uint16_t)*bufp << 8 | (uint16_t)reg;

    write_trans.length = num_bits;
    write_trans.tx_buffer = &full_command;
    write_trans.rx_buffer = &write_buffer;
    printf("Being written: 0x%04X\n", *(uint16_t*)(write_trans.tx_buffer));
    spi_device_transmit(spi, &write_trans);

    return 0;
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
    
    *bufp = 0;

    uint8_t read_reg;
    uint8_t arr[1+len];
    memset(arr, 0xFF, sizeof(uint8_t)*(1+len));
    int num_bits = 8 + 8*len;

    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));
    trans.length = num_bits;
    trans.tx_buffer = &read_reg;
    trans.rx_buffer = &arr;


    read_reg = reg | SPI_READ_MASK;
    spi_device_transmit(spi, &trans);

    for(int i = 0; i < len+1; i++){
        *(bufp+i) = arr[1 + i];
    }
    // bufp += 1;

    return 0;
}

static void platform_init(void) {
    
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = CLOCK_SPEED,     // Clock out at 10 KHz
        .mode = 3,                  // SPI mode 3
        .spics_io_num = 5,          // CS pin
        .queue_size = 7,            // Queue 7 transactions at a time
    };

    spi_bus_config_t buscfg = {
        .miso_io_num = 19,
        .mosi_io_num = 23,
        .sclk_io_num = 18,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };

    spi_bus_initialize(VSPI_HOST, &buscfg, 1);

    spi_bus_add_device(VSPI_HOST, &devcfg, &spi);
}

static void platform_delay(uint32_t ms) {
    vTaskDelay(ms/10);
}
