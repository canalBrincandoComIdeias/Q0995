/*
*    AUTOR:		  BrincandoComIdeias
*    APRENDA: 	https://cursodearduino.net/
*    SKETCH:    Controle de Umidade do Ar
*    DATA:		  14/03/2023
*/

// Placa Generic ESP8266 v3.0.2
#include "config.h"

// INSTANCIANDO OBJETOS
AdafruitIO_Feed *feedUmidade = io.feed("umidade");
AdafruitIO_Feed *feedDesumidficador = io.feed("desumidificador");
AdafruitIO_Feed *feedUmidificador = io.feed("umidificador");
AdafruitIO_Feed *feedAutomatico = io.feed("automatico");

// DECLARAÇÃO DE VARIÁVEIS
float umidade = 0.0;

int desumidificador = -1; 
int umidificador = -1;

void setup() {
  Serial.begin(9600);

  // CONECTA NO WIFI + ADAFRUITIO
  io.connect();

  // CONFIGURA AS FUNÇÕES QUE SERÃO EXECUTA AO RECEBER COMANDO DO MQTT
  feedUmidificador->onMessage(handleUmidificador);
  feedDesumidficador->onMessage(handleDesumidificador);
  feedAutomatico->onMessage(handleAutomatico);

  // PISCA O LED ENQUANTO TENTA CONECTAR
  while (io.status() < AIO_CONNECTED) {
    delay(500);
  }

  feedAutomatico->save(0);
  feedUmidificador->save(0);
  feedDesumidficador->save(0);

  // Apaga o Buffer do Serial.
  String str = Serial.readString();
}

void loop() {
  io.run();

  // VERIFICA SE TEM ALGUMA INFORMAÇÃO NO SERIAL
  if (Serial.available()) {
    // INCIA A STRING VAZIA
    String recebido = "";

    // LÊ O BUFFER ATÉ O PRÓXIMO ';'
    recebido = Serial.readStringUntil(';');
    umidade = recebido.toFloat();

    // LÊ O BUFFER ATÉ O PRÓXIMO ';'
    recebido = Serial.readStringUntil(';');
    umidificador = recebido.toInt();

    // LÊ O BUFFER ATÉ O PRÓXIMO';'
    recebido = Serial.readStringUntil(';');
    desumidificador = recebido.toInt();

    // ENVIA PARA O ADAFRUIT IO
    if (!isnan(umidade)) {
      feedUmidade->save(umidade);
      feedUmidificador->save(umidificador);
      feedDesumidficador->save(desumidificador);
    }
  }
}

void handleDesumidificador(AdafruitIO_Data *data) {
  char *val = data->value();

  if (*val == '1') {
    Serial.print("D;");
  } else if (*val == '0') {
    Serial.print("d;");
  }
}

void handleUmidificador(AdafruitIO_Data *data) {
  char *val = data->value();

  if (*val == '1') {
    Serial.print("U;");
  } else if (*val == '0') {
    Serial.print("u;");
  }
}

void handleAutomatico(AdafruitIO_Data *data) {
  char *val = data->value();

  if (*val == '1') {
    Serial.print("A;");
  } else if (*val == '0') {
    Serial.print("a;");
  }
}
