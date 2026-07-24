# Protocolo SPI del Display e-Paper

## Estructura de comandos

Todos los datos se envian mediante `sendIndexData()`:

```
DC=LOW, CS=LOW -> index (1B) -> CS=HIGH   (comando)
DC=HIGH, CS=LOW -> data[i] -> CS=HIGH     (dato, repetido por cada byte)
```

CS se pulsa por cada byte (no se mantiene bajo durante toda la transaccion).

## Secuencia de inicio (COG_initial)

1. Configurar pines: BUSY=IN, DC=OUT, RESET=OUT, CS=OUT
2. DC=HIGH, RESET=HIGH, CS=HIGH
3. delay(5ms)
4. Reset: HIGH->5ms->LOW->10ms->HIGH->5ms->CS=HIGH->5ms
5. Soft reset: cmd 0x00, data 0x0E, esperar BUSY=HIGH
6. Input Temperature: cmd 0xE5, data 25C
7. Active Temperature: cmd 0xE0, data reg[2]
8. PSR (Panel Setting Register): cmd 0x00, data reg[3..4]

## Comandos principales

| Cmd   | Nombre | Datos       | Descripcion              |
|-------|--------|-------------|--------------------------|
| 0x00  | PSR    | 2 bytes     | Configuracion de panel   |
| 0x02  | POF    | 0 bytes     | Apagar DC/DC             |
| 0x04  | PON    | 0 bytes     | Encender DC/DC           |
| 0x10  | DTM1   | image_size  | Enviar frame 1 (BW)     |
| 0x12  | DRF    | 0 bytes     | Display Refresh          |
| 0x13  | DTM2   | image_size  | Enviar frame 2 (BWR)    |
| 0xE0  | ACT    | 1 byte      | Active Temperature       |
| 0xE5  | ITC    | 1 byte      | Input Temperature        |

## Tamanos de imagen

| Pantalla | Resolucion  | image_data_size |
|----------|-------------|-----------------|
| 2.13"    | 212 x 104   | 2756 bytes      |
| 2.66"    | 296 x 152   | 5624 bytes      |
| 1.54"    | 200 x 200   | 5000 bytes      |

## globalUpdate()

```
sendIndexData(0x10, data1s, image_data_size)   -- Frame BW
sendIndexData(0x13, data2s, image_data_size)   -- Frame BWR
DCDC_powerOn()    -- cmd 0x04, esperar BUSY
displayRefresh()  -- cmd 0x12, esperar BUSY
```

## Tiempos de espera (busy polling)

| Operacion     | Timeout  |
|---------------|----------|
| softReset     | 5000 ms  |
| DCDC_powerOn  | 10000 ms |
| displayRefresh| 60000 ms |
| COG_powerOff  | 5000 ms  |
