/*
 * test_ds3231.c
 *
 * Pruebas Ceedling/Unity del driver DS3231.
 * El bus I2C se sustituye con CMock (mock_i2c_drv).
 *
 * Ejecutar desde la raíz del proyecto:
 *   ceedling test:all
 */

#include "unity.h"
#include "ds3231.h"
#include "mock_i2c_drv.h"

void setUp(void)
{
	DS3231_DeInit();
}

void tearDown(void)
{
}

/* ------------------------------------------------------------------
 * 8 pruebas unitarias (módulo DS3231 aislado, I2C mockeado)
 * ------------------------------------------------------------------ */

void test_1_ApplyHourFormat_12h_PM_a_24h(void)
{
	/* 3:00 PM en 12 h → registro 0x63 → 15:00 en 24 h (0x15) */
	uint8_t hour12pm = 0x03u | 0x40u | 0x20u;
	uint8_t hour24;

	hour24 = DS3231_ApplyHourFormat(hour12pm, DS3231_FORMAT_24H);

	TEST_ASSERT_EQUAL_HEX8(0x15u, hour24);
	TEST_ASSERT_EQUAL_HEX8(0x00u, hour24 & 0x40u);
}

void test_2_ApplyHourFormat_24h_medianoche_a_12h(void)
{
	/* 00:00 en 24 h → 12:00 AM en 12 h (0x12 | bit12h) */
	uint8_t hour24 = 0x00u;
	uint8_t hour12;

	hour12 = DS3231_ApplyHourFormat(hour24, DS3231_FORMAT_12H);

	TEST_ASSERT_EQUAL_HEX8(0x12u, hour12 & 0x1Fu);
	TEST_ASSERT_TRUE((hour12 & 0x40u) != 0u);
	TEST_ASSERT_EQUAL_HEX8(0x00u, hour12 & 0x20u);
}

void test_3_Init_sin_dispositivo_queda_no_disponible(void)
{
	i2cDeviceReady_ExpectAndReturn(DS3231_ADDR, false);

	DS3231_Init(DS3231_FORMAT_24H);

	TEST_ASSERT_FALSE(DS3231_IsAvailable());
}

void test_4_ReadTime_24h_decodifica_BCD(void)
{
	uint8_t raw[3] = {0x45u, 0x30u, 0x15u};	/* 15:30:45, modo 24 h */
	DS3231_DateTime_t dt = {0};

	i2cDeviceReady_ExpectAndReturn(DS3231_ADDR, true);
	i2cMemRead_ExpectAndReturn(DS3231_ADDR, 0x00u, NULL, 3u, true);
	i2cMemRead_IgnoreArg_data();
	i2cMemRead_ReturnArrayThruPtr_data(raw, 3);

	DS3231_ReadTime(&dt);

	TEST_ASSERT_EQUAL_UINT8(45u, dt.second);
	TEST_ASSERT_EQUAL_UINT8(30u, dt.minute);
	TEST_ASSERT_EQUAL_UINT8(15u, dt.hour);
	TEST_ASSERT_EQUAL_INT(DS3231_FORMAT_24H, dt.hourFormat);
}

void test_5_ReadTime_12h_PM(void)
{
	uint8_t raw[3] = {0x00u, 0x00u, (uint8_t)(0x03u | 0x40u | 0x20u)};
	DS3231_DateTime_t dt = {0};

	i2cDeviceReady_ExpectAndReturn(DS3231_ADDR, true);
	i2cMemRead_ExpectAndReturn(DS3231_ADDR, 0x00u, NULL, 3u, true);
	i2cMemRead_IgnoreArg_data();
	i2cMemRead_ReturnArrayThruPtr_data(raw, 3);

	DS3231_ReadTime(&dt);

	TEST_ASSERT_EQUAL_UINT8(3u, dt.hour);
	TEST_ASSERT_EQUAL_INT(DS3231_FORMAT_12H, dt.hourFormat);
	TEST_ASSERT_EQUAL_INT(DS3231_PM, dt.amPm);
}

void test_6_ReadDate_decodifica_BCD(void)
{
	uint8_t raw[4] = {0x04u, 0x15u, 0x10u, 0x26u};	/* jueves 15/10/26 */
	DS3231_DateTime_t dt = {0};

	i2cDeviceReady_ExpectAndReturn(DS3231_ADDR, true);
	i2cMemRead_ExpectAndReturn(DS3231_ADDR, 0x03u, NULL, 4u, true);
	i2cMemRead_IgnoreArg_data();
	i2cMemRead_ReturnArrayThruPtr_data(raw, 4);

	DS3231_ReadDate(&dt);

	TEST_ASSERT_EQUAL_UINT8(4u, dt.weekDay);
	TEST_ASSERT_EQUAL_UINT8(15u, dt.day);
	TEST_ASSERT_EQUAL_UINT8(10u, dt.month);
	TEST_ASSERT_EQUAL_UINT8(26u, dt.year);
}

void test_7_WriteTime_24h_escribe_BCD(void)
{
	DS3231_DateTime_t dt = {0};
	uint8_t expected[3] = {0x45u, 0x30u, 0x15u};

	dt.hour = 15u;
	dt.minute = 30u;
	dt.second = 45u;
	dt.hourFormat = DS3231_FORMAT_24H;

	i2cDeviceReady_ExpectAndReturn(DS3231_ADDR, true);
	i2cMemWrite_ExpectWithArrayAndReturn(DS3231_ADDR, 0x00u, expected, 3, 3u, true);
	i2cMemRead_ExpectAndReturn(DS3231_ADDR, 0x0Fu, NULL, 1u, true);
	i2cMemRead_IgnoreArg_data();
	i2cMemWrite_ExpectAndReturn(DS3231_ADDR, 0x0Fu, NULL, 1u, true);
	i2cMemWrite_IgnoreArg_data();

	DS3231_WriteTime(&dt);

	TEST_ASSERT_TRUE(DS3231_IsAvailable());
}

void test_8_ReadTime_NULL_no_llama_I2C(void)
{
	DS3231_ReadTime(NULL);
	/* CMock falla si se llamó a alguna función I2C no esperada */
}

/* ------------------------------------------------------------------
 * 1 prueba de integración: WriteTime + ReadTime sobre el mismo bus mock
 * ------------------------------------------------------------------ */

void test_integracion_WriteTime_luego_ReadTime(void)
{
	DS3231_DateTime_t in = {0};
	DS3231_DateTime_t out = {0};
	uint8_t bus[3] = {0x45u, 0x30u, 0x15u};	/* 15:30:45 BCD */

	in.hour = 15u;
	in.minute = 30u;
	in.second = 45u;
	in.hourFormat = DS3231_FORMAT_24H;

	i2cDeviceReady_ExpectAndReturn(DS3231_ADDR, true);
	i2cMemWrite_ExpectWithArrayAndReturn(DS3231_ADDR, 0x00u, bus, 3, 3u, true);
	i2cMemRead_ExpectAndReturn(DS3231_ADDR, 0x0Fu, NULL, 1u, true);
	i2cMemRead_IgnoreArg_data();
	i2cMemWrite_ExpectAndReturn(DS3231_ADDR, 0x0Fu, NULL, 1u, true);
	i2cMemWrite_IgnoreArg_data();

	DS3231_WriteTime(&in);
	TEST_ASSERT_TRUE(DS3231_IsAvailable());

	i2cMemRead_ExpectAndReturn(DS3231_ADDR, 0x00u, NULL, 3u, true);
	i2cMemRead_IgnoreArg_data();
	i2cMemRead_ReturnArrayThruPtr_data(bus, 3);

	DS3231_ReadTime(&out);

	TEST_ASSERT_EQUAL_UINT8(in.hour, out.hour);
	TEST_ASSERT_EQUAL_UINT8(in.minute, out.minute);
	TEST_ASSERT_EQUAL_UINT8(in.second, out.second);
	TEST_ASSERT_EQUAL_INT(DS3231_FORMAT_24H, out.hourFormat);
}


/* 10. Inicialización normal: oscilador habilitado y formato 24 h. */
void test_10_Init_dispositivo_disponible_formato24(void)
{
    uint8_t control = 0x00u; /* EOSC = 0 */
    uint8_t hour = 0x15u;   /* 15:00 ya en formato 24 h */

    i2cDeviceReady_ExpectAndReturn(DS3231_ADDR, true);

    i2cMemRead_ExpectAndReturn(DS3231_ADDR, 0x0Eu, NULL, 1u, true);
    i2cMemRead_IgnoreArg_data();
    i2cMemRead_ReturnArrayThruPtr_data(&control, 1);

    i2cMemRead_ExpectAndReturn(DS3231_ADDR, 0x02u, NULL, 1u, true);
    i2cMemRead_IgnoreArg_data();
    i2cMemRead_ReturnArrayThruPtr_data(&hour, 1);

    i2cMemWrite_ExpectWithArrayAndReturn(DS3231_ADDR, 0x02u,
                                         &hour, 1, 1u, true);

    DS3231_Init(DS3231_FORMAT_24H);
    TEST_ASSERT_TRUE(DS3231_IsAvailable());
}

/* 11. Escritura de fecha: 15/10/2026 se codifica en BCD. */
void test_11_WriteDate_escribe_BCD_correctamente(void)
{
    DS3231_DateTime_t dt = {0};
    uint8_t expected[4] = {0x04u, 0x15u, 0x10u, 0x26u};
    dt.weekDay = 4u;
    dt.day = 15u;
    dt.month = 10u;
    dt.year = 26u;

    i2cDeviceReady_ExpectAndReturn(DS3231_ADDR, true);
    i2cMemWrite_ExpectWithArrayAndReturn(DS3231_ADDR, 0x03u,
                                         expected, 4, 4u, true);

    DS3231_WriteDate(&dt);
    TEST_ASSERT_TRUE(DS3231_IsAvailable());
}

/* 12. Lectura de temperatura interna: 25.25 C. */
void test_12_GetTemperature_devuelve_25_25C(void)
{
    uint8_t raw[2] = {0x19u, 0x40u}; /* 25 + 0.25 */
    float temperature = -1.0f;

    i2cDeviceReady_ExpectAndReturn(DS3231_ADDR, true);
    i2cMemRead_ExpectAndReturn(DS3231_ADDR, 0x11u, NULL, 2u, true);
    i2cMemRead_IgnoreArg_data();
    i2cMemRead_ReturnArrayThruPtr_data(raw, 2);

    TEST_ASSERT_TRUE(DS3231_GetTemperature(&temperature));
    TEST_ASSERT_TRUE(temperature > 25.24f && temperature < 25.26f);
}

/* 13. Fallo de lectura I2C: pasa a no disponible y conserva datos. */
void test_13_ReadTime_fallo_I2C_marca_no_disponible(void)
{
    DS3231_DateTime_t dt = {0};
    dt.hour = 7u;

    i2cDeviceReady_ExpectAndReturn(DS3231_ADDR, true);
    i2cMemRead_ExpectAndReturn(DS3231_ADDR, 0x00u, NULL, 3u, false);
    i2cMemRead_IgnoreArg_data();

    DS3231_ReadTime(&dt);
    TEST_ASSERT_FALSE(DS3231_IsAvailable());
    TEST_ASSERT_EQUAL_UINT8(7u, dt.hour);
}


