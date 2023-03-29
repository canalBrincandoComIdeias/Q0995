/*
*    AUTOR:		  BrincandoComIdeias
*    APRENDA: 	https://cursodearduino.net/
*    SKETCH:    Controle de Umidade do Ar
*    DATA:		  14/03/2023
*/

// INCLUSÃO DE BIBLIOTECAS
#include "DHT.h"
#include <MicroLCD.h>

// INCLUSÃO DE ARQUIVOS
#include "icones.h"

// DEFINIÇÕES DE PINOS
#define pinDHT 10
#define pinUmid 11
#define pinDesumid 9

// DEFINIÇÕES
#define UR_MAX 60    // Umidade Relativa Maxima
#define UR_IDEAL 55  // Umidade Alvo
#define UR_MIN 50    // Umidade Relativa Minima

#define INTERVALO_LEITURA 10000  // Intervalo minimo de leitura do DHT22 ~2s \
                                 // Intervalo minimo do AdafruitIO ~10s

#define LIGADO LOW
#define DESLIGADO HIGH

#define AUTOMATICO true
#define MANUAL false

// INSTANCIANDO OBJETOS
DHT dht(pinDHT, DHT22);
LCD_SH1106 lcd;  //LCD_SSD1306   //LCD_SH1106   //LCD_PCD8544

// DECLARAÇÃO DE VARIÁVEIS
bool atualiza = false;
float umidade = 55.0;
float umidadeAnt = 55.0;

bool estadoUmidificador = false;
bool estadoDesumidificador = false;

bool estadoUmidificadorAnt = true;
bool estadoDesumidificadorAnt = true;

int estado = 0;  // 0 = Ar ideal | -1 = Umidificador ligado | 1 = Ar condicionado Ligado
unsigned long controleLeitura;

bool modo = MANUAL;
unsigned long controleModo;

void setup() {
  Serial.begin(9600);
  Serial3.begin(9600);

  // Configura os pinos como OUTPUT
  pinMode(pinDesumid, OUTPUT);
  pinMode(pinUmid, OUTPUT);

  digitalWrite(pinUmid, HIGH);
  digitalWrite(pinDesumid, HIGH);

  // Inicializa o Sensor
  dht.begin();

  // Inicializa o Display e limpa a tela
  lcd.begin();
  lcd.clear();

  // Posiciona o Cursos e Imprime o Logo
  lcd.setCursor(39, 1);
  lcd.draw(logo, 48, 48);

  // Espera alguns segundos com o Logo na tela e limpa o Serial
  delay(8000);
  if (Serial3.available()) String str = Serial3.readString();

  // Remove o Logo da tela
  lcd.clear();

  // Posiciona o Cursor e Imprime o Icone de Umidade Relativa
  lcd.setCursor(95, 0);
  lcd.draw(humidity, 32, 32);

  // Imprime o estado inicial dos equipamentos
  lcd.setCursor(0, 6);
  lcd.println(F("Umidif.: Desligado"));
  lcd.println(F("Desumid: Desligado"));

  // Mostra o modo em Manual
  lcd.setCursor(115, 6);
  lcd.setFontSize(FONT_SIZE_XLARGE);
  lcd.print(F("M"));
  lcd.setFontSize(FONT_SIZE_SMALL);
}

void loop() {
  if (Serial3.available()) {
    String comando = "";

    // LÊ O BUFFER ATÉ O PRÓXIMO';'
    comando = Serial3.readStringUntil(';');

    //DEBUG
    Serial.println(comando);

    if (isvalid(comando)) {

      if (comando == "a") {
        modo = MANUAL;

        lcd.setCursor(115, 6);
        lcd.setFontSize(FONT_SIZE_XLARGE);
        lcd.print(F("M"));
        lcd.setFontSize(FONT_SIZE_SMALL);
      }

      if (comando == "A") {
        modo = AUTOMATICO;
        estado = 0;

        lcd.setCursor(115, 6);
        lcd.setFontSize(FONT_SIZE_XLARGE);
        lcd.print(F("A"));
        lcd.setFontSize(FONT_SIZE_SMALL);
      }

      if (modo == MANUAL) {
        if (comando == "u") {
          estadoUmidificador = false;
        }
        if (comando == "U") {
          estadoUmidificador = true;
        }
        if (comando == "d") {
          estadoDesumidificador = false;
        }
        if (comando == "D") {
          estadoDesumidificador = true;
        }
      } else {                                       //se estiver em automatico
        if (comando == "u" && estadoUmidificador) {  //se pediu para desligar, retorna o estado
          atualiza = true;                           //Para forçar retorno do valor para o Adafruit
        }
        if (comando == "U" && !estadoUmidificador) {  //se pediu para ligar, retorna o estado
          atualiza = true;                            //Para forçar retorno do valor para o Adafruit
        }
        if (comando == "d" && estadoDesumidificador) {
          atualiza = true;  //Para forçar retorno do valor para o Adafruit
        }
        if (comando == "D" && !estadoDesumidificador) {
          atualiza = true;  //Para forçar retorno do valor para o Adafruit
        }
      }
    }
  }

  if ((millis() - controleLeitura) > INTERVALO_LEITURA) {
    float leitura = dht.readHumidity();
    controleLeitura = millis();

    lcd.setCursor(45, 1);

    if (!isnan(leitura)) {  // Confere se recebeu algum valor do sensor
      umidade = leitura;

      if (umidade != umidadeAnt) {
        lcd.setFontSize(FONT_SIZE_XLARGE);
        lcd.print(umidade);
        lcd.setFontSize(FONT_SIZE_SMALL);
        atualiza = true;
      }
    } else {
      lcd.setCursor(0, 6);
      lcd.println(F("                  "));
      lcd.println(F("-->Falha no DHT<--"));
      delay(1000);
    }

    if (modo == AUTOMATICO) {
      switch (estado) {
        case 0:                    // Leitura Anterior = Indicando Umidade ideal | Umidificador e Ar desligados
          if (umidade < UR_MIN) {  // Se Leitura Atual = Indicando Ar muito Seco
            estado = -1;
            estadoUmidificador = true;
            atualiza = true;
          } else if (umidade > UR_MAX) {  // Se Leitura Atual = Indicando Ar muito umido
            estado = 1;
            estadoDesumidificador = true;
            atualiza = true;
          }
          break;

        case -1:                      // Leitura Anterior = Indicando Umidade baixa | Umidificador ligado
          if (umidade >= UR_IDEAL) {  // Se Leitura Atual = Indicando Umidade subiu para o nível ideal
            estado = 0;
            estadoUmidificador = false;
            atualiza = true;
          }
          break;

        case 1:                       // Leitura Anterior = Indicando Umidade alta | Ar ligado
          if (umidade <= UR_IDEAL) {  // Se Leitura Atual = Indicando Umidade abaixou para o nível ideal
            estado = 0;
            estadoDesumidificador = false;
            atualiza = true;
          }
          break;
      }
    }
  }

  // VERIFICA SE HOUVE VARIAÇÃO NA LEITURA OU NO ESTADO DOS RELÉS
  if ((umidade != umidadeAnt) || (estadoUmidificador != estadoUmidificadorAnt) || (estadoDesumidificador != estadoDesumidificadorAnt)) {
    // ATUALIZA O ÍCONE NO DISPLAY
    if (umidade >= UR_MIN && umidade <= UR_MAX) {
      lcd.setCursor(0, 0);
      lcd.draw(good, 32, 32);
    } else {
      lcd.setCursor(0, 0);
      lcd.draw(bad, 32, 32);
    }

    lcd.setCursor(0, 6);
    lcd.print(F("Umidif.: "));
    if (estadoUmidificador) {
      lcd.println(F("Ligado   "));
      digitalWrite(pinUmid, LIGADO);
    } else {
      lcd.println(F("Desligado"));
      digitalWrite(pinUmid, DESLIGADO);
    }

    lcd.print(F("Desumid: "));
    if (estadoDesumidificador) {
      lcd.println(F("Ligado   "));
      digitalWrite(pinDesumid, LIGADO);
    } else {
      lcd.println(F("Desligado"));
      digitalWrite(pinDesumid, DESLIGADO);
    }
  }

  if (atualiza) {  // ENVIA PARA O ESP AS INFORMAÇÕES ATUAIS
    atualiza = false;
    Serial3.print(umidade);
    Serial3.print(';');
    Serial3.print(estadoUmidificador);
    Serial3.print(';');
    Serial3.print(estadoDesumidificador);
    Serial3.print(';');
  }

  umidadeAnt = umidade;
  estadoUmidificadorAnt = estadoUmidificador;
  estadoDesumidificadorAnt = estadoDesumidificador;
}

bool isvalid(String c) {
  int temp = c.charAt(0);
  temp = toupper(temp);
  return temp == 'D' || temp == 'U' || temp == 'A';
}