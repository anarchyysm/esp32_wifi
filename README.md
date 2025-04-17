# esp32_wifi
esp32wifi highboy

## Documentação

#### Instalar esp-idf

1. `./install.sh`
2. `source export.sh` Para habilitar as funções do idf.py
3. `idf.py set-target esp32s3` ou `idf.py menuconfig`
4. `idf.py -p PORT build` // buildar o programa C
5. `idf.py -p PORT flash` // enviar pro esp32-s3
6. `idf.py -p PORT monitor` // monitorar o que ta acontendo

### Arquitetura
![Video exemplo](./arquitetura.png)
