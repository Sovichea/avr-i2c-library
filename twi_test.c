/*
* TWI_Test.c
*
* Created: 08-Jun-19 10:06:47 AM
* Author : TEP SOVICHEA
*/

#include "uart.h"
#include "twi_master.h"
#include <avr/io.h>
#include <util/delay.h>

/************************************************************************/
/*							Initializations                             */
/************************************************************************/

#define MPU6050_ADDR	0x68

/* MPU6050 register address */
#define ACCEL_XOUT_H	0x3B
#define ACCEL_XOUT_L	0x3C
#define ACCEL_YOUT_H	0x3D
#define ACCEL_YOUT_L	0x3E
#define ACCEL_ZOUT_H	0x3F
#define ACCEL_ZOUT_L	0x40
#define PWR_MGMT_1		0x6B

typedef struct
{
	float x;
	float y;
	float z;
} mpu_data_t;


/************************************************************************/
/*							Prototype functions                         */
/************************************************************************/

void ERROR_CHECK(ret_code_t error_code);
void mpu_init(void);
void mpu_get_accel_raw(mpu_data_t* mpu_data);
void mpu_get_accel(mpu_data_t* mpu_data);


/************************************************************************/
/*							Function definitions                        */
/************************************************************************/

void ERROR_CHECK(ret_code_t error_code)
{
	if (error_code != SUCCESS)
	{
		/* Print error code and loop indefinitely until reset */
		printf(BR "App error! error_code = 0x%02X\n" RESET, error_code);
		while (1); // loop indefinitely
	}
}


void mpu_init(void)
{
	ret_code_t error_code;
	puts("Write 0 to PWR_MGMT_1 reg to wakeup MPU.");
	uint8_t data[2] = {PWR_MGMT_1, 0};
	error_code = tw_master_transmit(MPU6050_ADDR, data, sizeof(data), false);
	ERROR_CHECK(error_code);
}


void mpu_get_accel_raw(mpu_data_t* mpu_data)
{
	ret_code_t error_code;
	/* 2 registers for each of accel x, y and z data */
	uint8_t data[6];
	
	data[0] = ACCEL_XOUT_H;
	error_code = tw_master_transmit(MPU6050_ADDR, data, 1, true);
	ERROR_CHECK(error_code);
	
	error_code = tw_master_receive(MPU6050_ADDR, data, sizeof(data));
	ERROR_CHECK(error_code);
	
	/* Default accel config +/- 2g */
	mpu_data->x = (int16_t)(data[0] << 8 | data[1]) / 16384.0;
	mpu_data->y = (int16_t)(data[2] << 8 | data[3]) / 16384.0;
	mpu_data->z = (int16_t)(data[4] << 8 | data[5]) / 16384.0;
}


void mpu_get_accel(mpu_data_t* mpu_data)
{
	mpu_get_accel_raw(mpu_data);
	mpu_data->x = mpu_data->x * 9.81;
	mpu_data->y = mpu_data->y * 9.81;
	mpu_data->z = mpu_data->z * 9.81;
}


/************************************************************************/
/*							Main application                            */
/************************************************************************/

int main(void)
{
	/* Initialize UART */
	uart_init(250000); // bps
	cli_reset();
	puts(BY "Initializing TWI_Test Project...\n" RESET);
	
	/* Initialize project configuration */
	tw_init(TW_FREQ_400K, true); // set I2C Frequency, enable internal pull-up
	mpu_init();
	mpu_data_t accel;
	
	puts(BG CURSOR_RIGHT("14")
	"--------------- Application Started ---------------\n" RESET);
	
	while (1)
	{
		puts("Read accelerometer data.");
		mpu_get_accel(&accel);
		printf("Accel X: %5.2f\n", accel.x);
		printf("Accel Y: %5.2f\n", accel.y);
		printf("Accel Z: %5.2f\n", accel.z);
		_delay_ms(200);
	}
}
