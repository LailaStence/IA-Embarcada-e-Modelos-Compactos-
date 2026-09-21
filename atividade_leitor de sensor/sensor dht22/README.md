# Atividade Avaliativa Prática 2/6 — Leitura de sensor

Projeto em **C com ESP-IDF** para o **ESP32-S3**, utilizando o sensor **DHT11** simulado no Wokwi. O firmware lê temperatura e umidade a cada dois segundos e imprime os valores no monitor serial.

## 1. Materiais e ligações

| DHT11 | ESP32-S3 DevKitC-1 | Função |
|---|---|---|
| VCC | 3V3 | Alimentação |
| GND | GND.1 | Referência |
| SDA/DATA | GPIO 4 | Dados bidirecionais |

O DHT11 do Wokwi já possui o pull-up necessário para a simulação. O código também habilita o pull-up interno do GPIO 4.

## 2. Pré-requisitos

1. Instalar o **Visual Studio Code**.
2. Instalar e configurar a extensão **Espressif IDF**.
3. Instalar a extensão **Wokwi for VS Code**.
4. Criar uma conta em [wokwi.com](https://wokwi.com/) e configurar o token na extensão Wokwi quando solicitado.
5. Selecionar `ESP32-S3` como alvo do projeto.

## 3. Compilação pelo ESP-IDF

No terminal do VS Code, dentro desta pasta:

```bash
idf.py set-target esp32s3
idf.py build
```

O build deve terminar sem erros e gerar `build/atividade_sensor_dht11.elf` e `build/atividade_sensor_dht11.bin`.

## 4. Simulação no Wokwi

1. Abra esta pasta no VS Code.
2. Pressione `F1` e execute **Wokwi: Start Simulator**.
3. O arquivo `diagram.json` apresenta o ESP32-S3 e o DHT11 com as três conexões descritas acima.
4. Abra o **Wokwi Serial Monitor**.
5. A saída esperada é semelhante a:

```text
I (xxx) DHT11: Atividade Pratica 2/6 - Leitura de sensor
I (xxx) DHT11: ESP32-S3 + DHT11 | GPIO4 | periodo=2000 ms
I (xxx) DHT11: Leitura: temperatura=24 C | umidade=55 %
I (xxx) DHT11: Leitura: temperatura=24 C | umidade=55 %
```

No simulador, altere os atributos `temperature` e `humidity` do DHT11 no `diagram.json` para demonstrar que as leituras acompanham o sensor simulado.

## 5. Arquivos da entrega

- `main/main.c`: inicialização do GPIO, protocolo de 40 bits do DHT11, checksum e impressão serial.
- `diagram.json`: circuito Wokwi.
- `wokwi.toml`: associação do firmware/ELF compilado ao simulador.
- `sdkconfig.defaults`: alvo e configurações padrão do ESP32-S3.
- `evidencia-monitor-serial.svg`: modelo visual da captura da saída esperada; substitua por uma captura real do monitor do VS Code antes da entrega final, se a instituição exigir screenshot do simulador.

## 6. Checklist de avaliação

- [ ] ESP-IDF configurado no VS Code.
- [ ] Conta/token do Wokwi configurado.
- [ ] Circuito com VCC, GND e DATA conferido.
- [ ] `idf.py build` concluído sem erros.
- [ ] Simulação iniciada pelo Wokwi.
- [ ] Monitor serial exibindo temperatura e umidade.
- [ ] Repositório Git criado e linkado na entrega.
