//************************************************
//      Código funcional para a varredura
// Vai para os fins de curso (FCs), depois vai para a posição início onde o plasma é ligado, depois inicia o tratamento.
// se o número de tratamentos for par o código vai terminar imediatamente abaixo da posição inicio.
// Porém se for ímpar o código vai terminar do outro lado da mesa. Em ambos os casos o código volta pra posição início para começar
// uma nova amostra.
// *NOTA: Esse código não usa a função movimenta pra tentar ir na diagonal do fim do tratamento até a posição inicio
//*NOTA2: o Y da posição inicio agora é 10, pois assim ele não aciona o FC enquanto está se movimentando, causando o travamento do código
//***************************************************************

#include <StepDriverCCS.h> //biblioteca stepDriver alterada
A4988CCS dx, dy;           // objetos referentes a cada um dos motores

unsigned int i; // variavel usada nos loops
int led1=12, led2=13, botao=A3; // pinos usados nos leds e no botão
int fcx=9, fcy=10;              // pinos dos fins de curso (FC)
int stepx=2, dirx=5; //distx=20;  // pinos de controle do número de passos e direçao do motor X
int stepy=3, diry=6; //disty=50;  // pinos de controle do número de passos e direçao do motor Y
int quant=20;//quantidade de varreduras
int vel=700; //velocidade da varredura, quanto menor mais rápido.      
float tam=.5; // tamanho da amostra (em polegadas) 
      tam=tam*25.4 //transforma tam para mm
float larg=10; // largura do feixe de plasma em mm que será usado como medida para fazer a sobreposição dos tratamentos
bool sent=HIGH; //variavel que indica o sentido do motor
                //HIGH eixo cresce, LOW eixo diminui, de acordo com a definição escrita nos motores. 
                //NAO ALTERAR A MENOS QUE A CONFIGURAÇÃO FÍSICA DOS MOTORES MUDE !!!

void setup() 
{
  //configuração dos pinos de controle dos motores
  dx.pinConfig(stepx,dirx);
  dy.pinConfig(stepy,diry);
  
  //Define quantos passos o motor realiza por volta
  dx.stepPerRound(200);
  dy.stepPerRound(200);
  
  //Define quantos passos o motor realiza por milimetro
  dx.stepPerMm(200);
  dy.stepPerMm(200); 

  //Define os pinos dos FCs de cada motor
  dx.endstopConfig(fcx);
  dy.endstopConfig(fcy);

  pinMode(botao,INPUT);  //botao de comando
  pinMode(led1,OUTPUT);  //leds de indicaçao (no lugar do monitor serial)
  pinMode(led2,OUTPUT);
 
  Serial.begin(9600);

  Serial.println("delay");
  delay(2000);
  //***indicaçao q comecou a funcionar, pisca os dois leds 3 vezes***
  for(i=1;i<4;i++)
  {
  digitalWrite(led1,HIGH);
  digitalWrite(led2,HIGH);
  delay(300);
  digitalWrite(led1,LOW);
  digitalWrite(led2,LOW);
  delay(300); 
  } 
 fc_inicio(); //vai para os FCs e depois para a posicção inicio

}

void loop() 
{  
  
   Serial.println("ligue o plasma e aperte o botao");
   leds(1); // 1 é o comando referente ao ligamento do plasma
            //led1: pisca, led2: apagado, até botao ser apertado
            
   Serial.println("varredura iniciada."); 
   leds(2); //led1: aceso, led2: apagado, até a varedura acabar
   varredura();
   digitalWrite(led1,LOW); //Apaga ambos os leds
   digitalWrite(led2,LOW); //  
   
  Serial.println("varredura finalizada, coloque nova amostra.");  
}

void fc_inicio()
{
 
 dx.motorMove(200,!sent,697); //200 é um valor qualquer maior do q a mesa(157mm)
 dy.motorMove(200,!sent,697); //pra garantir q ela corra até os FC's

 // vai até a posição "inicio"
 dx.motorMove(78.5-tam/2,sent,700); //coordenada x da posição inicio
 dy.motorMove(10,sent,700);         //coordenada y da posição inicio
 
}

//Função responsavel por acionar os leds durante cada etapa
void leds(int etapa)
{  
  if(etapa==1) //esperando ligar plasma
  {
   int estado=HIGH;
   bool aux=false;
     while(1&&!aux)
   {
     //led1: pisca, led2: apagado
    digitalWrite(led1,estado);
    digitalWrite(led2,LOW);
    aperte(&estado,&aux);
   }
   //mantem led aceso pra indicar q o plasma ja foi ligado
   digitalWrite(led1,HIGH); 
   
  }else if(etapa==2) //varredura iniciada
  {
   //led1: aceso, led2: apagado
     digitalWrite(led1,HIGH);
     digitalWrite(led2,LOW);
  
  }
}

//Fica verificando o botao e só para pra trocar o estado 
//dos leds quando o tempo de piscar (300ms) acabar. É uma 
//substituição ao o uso do blink com delay(), pois as vezes quando o botão era 
//apertado, ele não era acionado pois o código estava travado no delay()
void aperte(int *state, bool *aux)
{
  if(*state==HIGH)
  {*state=LOW;}
  else
  {*state=HIGH;}
  
  unsigned long anterior=millis();
  
  while((millis()-anterior <= 300) && !*aux)
  {
    if (!digitalRead(botao))
    {  
      *aux=!*aux;   
    }
  }
}

//funçao que faz o tratamento, inclusive verificando se a quantidade é par ou ímpar
void varredura () 
{
 const int n=2*((tam/larg)-1)+1;
 //n=numero de passos,precisa ser int pq é oq vai rodar no for.
 //Quando declara como int ele pega so a parte inteira(arredonda pra baixo), 
 //entao somar 1 garante q ele vai dar um passo a mais, ou seja,
 // arredonadar pra cima e cobrir a placa toda  

  //O bloco abaixo é responsável pela primeira passada do plasma
  dy.motorMove(((78.5-10)+tam/2),sent,vel);   // isso é a distancia em y da posiçao inicio
  delay(100);                                // ate o centro da placa + metade da placa. 
  dy.motorMove(tam,!sent,vel); //volta pro canto superior esquerdo da placa

  //No bloco abaixo o for() interno so faz uma varredura(de um lado a outro da placa)
 // ai se vezes for par ele vai e volta e se for impar so vai. Ai só precisa ficar trocando
 //a direcao do X, por isso o sentido=!sentido no segundo for. Note q "sentido" muda mas "sent" nao.
  bool sentido=sent;
  for (int j=0;j<quant;j++) // responsavel por ir e voltar de acordo com "quant"
  {
   for (i=0;i<n;i++) // responsavel pela varredura de um lado a outro da placa
   {
    dx.motorMove((larg/2),sentido,696); //da um passo pra direita (feixe/2 pra ter sobreposicao)
    dy.motorMove(tam,sent,vel); //corre na vertical pra baixo
    delay(100);
    dy.motorMove(tam,!sent,vel); //corre na vertical pra cima
    delay(100);
    Serial.println((n-i));
   }
   sentido=!sentido;
   
  }
   
   //*****Volta para a posição inicio*****
   if (quant%2) //quant é impar
   { 
    dy.motorMove(((78.5-10)+tam/2),!sent,696);
    dx.motorMove(tam,!sent,696);
   }
   else  //quant é par
   { dy.motorMove(((78.5-10)-tam/2),!sent,696); } //sobe ate a posicao "inicio"
   
}
//
//void movimenta(float tamanhoX,float tamanhoY,bool sentX,bool sentY,int velo)
//{
//  const int n_passos=10;
//  //const int n_passosY=10;
//  
//  float passoX=tamanhoX/n_passos; //quantidadde de passos para o eixo X
//  float passoY=tamanhoY/n_passos; //quantidadde de passos para o eixo Y
//  
//  for (i=0;i<n_passos;i++)
//  {
//     dx.motorMove(passoX,sentX,velo);
//     dy.motorMove(passoY,sentY,velo);
//  }
//}
