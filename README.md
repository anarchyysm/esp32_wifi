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

---

Definir o recurso: Decida o que o novo recurso faz e suas dependências (ex.: hardware, outros componentes).
    Planejar a estrutura: Escolha onde implementar (ex.: drivers/ para hardware, services/ para lógica, applications/ para funcionalidades, core/ para coordenação).
    Criar o driver (se necessário): Desenvolva o componente de baixo nível para o hardware (ex.: GPIO, I2C) em drivers/.
    Criar o serviço (se necessário): Implemente a lógica de alto nível em services/, usando o driver.
    Criar a aplicação (se necessário): Adicione a funcionalidade específica em applications/, integrando serviços.
    Atualizar o core: Modifique core/ para inicializar e coordenar o novo recurso (ex.: chamar init do serviço).
    Configurar o build: Atualize CMakeLists.txt (raiz e componentes) para incluir o novo componente.
    Testar: Compile (idf.py build), grave (idf.py flash), e monitore (idf.py monitor).

**Exemplo de implementação:**
Para implementar Bluetooth no ESP-IDF, usando a arquitetura modular do seu projeto (com drivers/, services/, applications/, core/), o fluxo é o mesmo fornecido anteriormente, mas adaptado ao Bluetooth. Aqui está a resposta simples e direta, indicando por qual parte da arquitetura começar e a sequência:

  1. Definir o recurso: Especificar o tipo de Bluetooth (ex.: BLE para sensores, Classic para áudio) e o que ele fará (ex.: enviar dados de um sensor).
  2. Começar pelo driver: Crie um componente em drivers/bluetooth/ para configurar e gerenciar o Bluetooth (ex.: inicializar BLE, configurar GAP/GATT).
  3. Criar o serviço: Desenvolva um componente em services/bluetooth_service/ para a lógica de alto nível (ex.: gerenciar conexões, enviar/receber dados).
  4. Criar a aplicação (se necessário): Adicione um componente em applications/ para casos de uso específicos (ex.: bluetooth_sensor para enviar dados de temperatura).
  5. Atualizar o core: Modifique core/kernel/ para inicializar o serviço Bluetooth (ex.: chamar bluetooth_service_init()).
  6. Configurar o build: Atualize CMakeLists.txt (raiz e componentes) para incluir drivers/bluetooth/ e services/bluetooth_service/, com dependências como bt.
  7. Testar: Compile (idf.py build), grave (idf.py flash), e monitore (idf.py monitor).

Por onde começar: Comece pelo driver em drivers/bluetooth/ para configurar o hardware Bluetooth usando as APIs do ESP-IDF (ex.: esp_bt_controller_init, esp_ble_gatt).
