#include "supabase.h"
#include "Adafruit_TCS34725.h"
#include <Wire.h>

// Inicializa o sensor com ganho padrão
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

uint8_t nextion = 8; // este valor será enviado pela nextion// nao deu certo //usar um botao

uint8_t start_motor = 6;

enum CorDetectada {
  SEM_OBJETO,
  VERMELHO,
  VERDE,
  AZUL,
  INDEFINIDO
};

// Variáveis para armazenar os valores RGB
int red = 0;
int green = 0;
int blue = 0;
int control = 0;

const int limiarSemObjeto = 105;
const int limiarSemObjeto2 = 70;

int sensores [9][2] = {
  { A1, 0},//sensor opitico pequeno
  { A3, 0},//sensor opitico medio
  { A0, 0},//sensor opitico grande         mudarpara o A0
  { A2, 0},//sensor opitico peça em curso
  { A4, 0},//sensor capacitivo
  { A5, 0},//sensor indutivo
  { 1, 0},//fim de curso1
  { 0, 0},//fim de curso2
  { A6, 0}///fim de curso3
};

int relays [4][2] = {
  { 9, 0},//cilindro1
  { 10, 0},//cilindro2
  { 2, 0},//cilindro3
  { 7, 0},//dobot_entrada
  { 3, 0} // Retorno_rampa3
};


int contadores[4][2] = {          //enviar está matriz para NEXTION
  { 1, 0},
  { 2, 0},
  { 3, 0},
  { 4, 0}
};


uint8_t limite = 8;         //limite é o pino 8

void setup() {

  Serial.begin(9600);

  while (!Serial);

  conectarWiFi();

  // put your setup code here, to run once:
  for( byte i = 0; i < 9; i++ )           //declara os sensores como entrada
  {
    pinMode(sensores[i][0], INPUT_PULLUP);
  }

  for( byte i = 0; i < 4; i++)
  {
    pinMode(relays[i][0], OUTPUT);
  }

  pinMode(limite, INPUT_PULLUP);

  pinMode(start_motor, OUTPUT);

  pinMode(nextion, INPUT_PULLUP);

  iniciarSensorCor();
}


void iniciarSensorCor(){

  if (tcs.begin()) {
    Serial.println("Sensor TCS34725 detectado!, Conectado com sucesso");
  } else {
    Serial.println("Sensor TCS34725 não encontrado. Verifique a conexão I2C.");
  }

}

void inicio()        //substituir por uma lógica com a nextion verdadeira
{
  if(digitalRead(nextion) == LOW)
  {
    digitalWrite(start_motor, HIGH);
    digitalWrite(relays[3][0], HIGH);
    relays[3][1] = 1;
    Serial.println("nextion enviou dado");
  }
  else
  {
    digitalWrite(start_motor, LOW);
    digitalWrite(relays[3][0], LOW);
    relays[3][1] = 0;
  }

  if(relays[3][1] == 1 && sensores[5][1] == 1)
  {
    digitalWrite(relays[3][0],LOW);
    relays[3][1] = 0;
  }
  else if(relays[3][1] == 1 && digitalRead(limite) == HIGH)
  {
    digitalWrite(relays[3][0],HIGH);
    relays[3][1] = 1;
}

}


void leitura_sensores()
{
  for( byte i = 0; i < 9; i++ )
  {                                                               //tpggle na matriz de sensores
    if(digitalRead(sensores[i][0]) == LOW && sensores[i][1] == 0) //altera o estado somente se estiver no zero
    {
      sensores[i][1] = 1;
    }
    if(digitalRead(sensores[3][0]) == HIGH)           //caso pino 4 entre em HIGH ele desliga
    {
      sensores[3][1] = 0;
    }
  }

}

void color() //configura o sensor
{
  tcs.getRawData(&red, &green, &blue, &control);
}

CorDetectada detectarCor( int r, int g, int b) //opera o sensor
{
  if (r > limiarSemObjeto && g > limiarSemObjeto && b > limiarSemObjeto)
    return SEM_OBJETO;

  if (r < limiarSemObjeto2 && g < limiarSemObjeto2 && b < limiarSemObjeto2)
    return SEM_OBJETO;

  if (r > g && r > b && r > 0.5) {
    Serial.println("Cor detectada: VERMELHO");
    return VERMELHO
  }
  
  if (g > r && g > b && g > 0.3) {
    Serial.println("Cor detectada: VERDE");
    return VERDE
  }
  
  if (b > r && b > g && b > 0.3) {
    Serial.println("Cor detectada: AZUL");
    return AZUL
  }

  return INDEFINIDO;
}

int defineCor()
{
  // Detecta a cor com base nos valores
  CorDetectada cor = detectarCor(red, green, blue);

  // Realiza ações com base na cor detectada
  switch (cor) {
    case SEM_OBJETO:
      Serial.println("Nenhuma cor detectada (sem objeto)");
      // digitalWrite(pinoledverm, HIGH);
      // digitalWrite(pinoledverd, HIGH);
      // digitalWrite(pinoledazul, HIGH);
      break;

    case VERMELHO:
      Serial.println("Vermelho detectado");
      // digitalWrite(pinoledverm, LOW);
      // digitalWrite(pinoledverd, HIGH);
      // digitalWrite(pinoledazul, HIGH);
      break;

    case VERDE:
      Serial.println("Verde detectado");
      // digitalWrite(pinoledverm, HIGH);
      // digitalWrite(pinoledverd, LOW);
      // digitalWrite(pinoledazul, HIGH);
      break;

    case AZUL:
      Serial.println("Azul detectado");
      // digitalWrite(pinoledverm, HIGH);
      // digitalWrite(pinoledverd, HIGH);
      // digitalWrite(pinoledazul, LOW);
      break;

    case INDEFINIDO:
      Serial.println("Cor indefinida");
      break;
  }

  return cor;
}



int gerar_codigo_peca() {
  int codigo = 0;
  int corDEF = defineCor();

  for (int i = 0; i < 6; i++) {
    if (sensores[i][1]) {
      codigo |= (1 << i);
    }
  }

  if(corDEF == AZUL)// bit 6 do código significa AZUL
  {
    codigo |= (1<<6);
  }
  else if(corDEF == VERDE) // bit 7 do código significa VERDE
  {
    codigo |= (1<<7);
  }
  else if(corDEF == VERMELHO) // bit 7 E 6 do código significa VERMELHO
  {
    codigo |= ((1<<7) | (1<<6));
  }
  else
  {
    codigo |= ((0<<7) | (0<<6));  //peça indefinida
  }
  return codigo;
}

void intervalo_de_peca()
{
  if(digitalRead(limite) == LOW)                //reseta as memórias após a peça percorrer o caminho de decissão
  {
    for( byte i = 0; i < 9; i++ )                //exceto o sensor de peça em curso
    {                                                               
      if(i != 3)
      {
        sensores[i][1] = 0;
      }
    }
    Serial.println("memórias resetadas");
  }
}

void fim_de_curso()       //implementa um contador
{

    if(sensores[6][1]) //FIM DE CURSO 1
    {
      contadores[0][1]++;
      relays[0][1] = 0;
      digitalWrite(relays[0][0], LOW);
      sensores[6][1] = 0;  //desliga a memória porque eu fiz um toggle na matriz sensores
    }
    else if(sensores[7][1]) //FIM DE CURSO 2
    {
      contadores[1][1]++;
      relays[1][1] = 0;
      digitalWrite(relays[1][0], LOW);
      sensores[7][1] = 0;
    }
    else if(sensores[8][1])  //FIM DE CURSO 3
    {
      contadores[2][1]++;
      relays[2][1] = 0;      // reseta a memória do estado da saída

      // RETORNO RAMPA 3
      digitalWrite(relays[2][0], LOW); // Desliga acionamento
      digitalWrite(relays[4][0], HIGH); // Aciona o Retorno

      digitalWrite(relays[2][0], LOW);
      sensores[8][1] = 0;
    }
}

void pecas_totais()   //soma total das peças da máquina que será enviado para a NEXTION
{ 
  int soma_total = 0;
  for(byte i = 0; i < 3; i++)
  {
    soma_total += contadores[i][1];
  }
  contadores[3][1] = soma_total;
}



void saida() {
  int codigo = gerar_codigo_peca();
  

  switch (codigo) {
    case 0b00011001: // rampa 1 (sem indutivo)   azul original : 0b01011001
    case 0b00111001: // rampa 1 (com indutivo)   azul original : 0b01111001
      if (relays[0][1] == 0) {
        relays[0][1] = 1;
        digitalWrite(relays[0][0], HIGH);
        Serial.println("Cilindro 1 ativado (rampa 1)");
        Serial.println(codigo, BIN);

        inserirNumeroNoSupabase(codigo, 1);
      }
      break;

    case 0b00111111: // rampa 2   verde      retornar ao original           //0b10111111
      if (relays[1][1] == 0) {
        relays[1][1] = 1;
        digitalWrite(relays[1][0], HIGH);
        Serial.println("Cilindro 2 ativado (rampa 2)");
        Serial.println(codigo, BIN);
        
        inserirNumeroNoSupabase(codigo, 2);
      }
      break;

    case 0b00011111: // rampa 3    vermelho               //0b11011111
      if (relays[2][1] == 0) {
        relays[2][1] = 1;
        digitalWrite(relays[2][0], HIGH);
        Serial.println("Cilindro 3 ativado (rampa 3)");
        Serial.println(codigo, BIN);
        
        inserirNumeroNoSupabase(codigo, 3);
      }
      break;

      case 0b11011111: //mudar para a peca a ser descartada
        Serial.println("Descarte ativado (descarte)");
        Serial.println(codigo, BIN);
        
        inserirNumeroNoSupabase(codigo, 4);

    default:
      Serial.print("Nenhum cilindro ativado. Código: ");
      Serial.println(codigo, BIN);  // Exibe o código binário
      
        
      break;
  }
}
void loop() 
{
  
  inicio();

  leitura_sensores();                                         //chama a leitura dos sensores

  //identificar_peca();

  color();

  defineCor();

  gerar_codigo_peca();

  intervalo_de_peca();

  fim_de_curso();

  pecas_totais();

  saida();

  Serial.println("Parâmetros: ");

  for( byte i = 0; i < 6; i++ )           //declara os sensores como entrada
  {
    Serial.print("Entrada  :");
    Serial.println(i);
    Serial.println(sensores[i][1]);
  }

  /*
  Serial.println("Parâmetros: ");
  Serial.println("contador1: ");
  Serial.println(contadores[0][1]); 
  Serial.println("---------------------");
  Serial.println("contador2: ");
  Serial.println(contadores[1][1]);
  Serial.println("---------------------");
  Serial.println("contador3: ");
  Serial.println(contadores[2][1]);
  Serial.println("---------------------");
  Serial.println("contador4: ");
  Serial.println(contadores[3][1]);
  Serial.println("---------------------");

   // Exibe no monitor serial
  Serial.print("Vermelho: ");
  Serial.print(red);
  Serial.print("  Verde: ");
  Serial.print(green);
  Serial.print("  Azul: ");
  Serial.println(blue);
  */
  

  Serial.println();

  delay(500);

}
