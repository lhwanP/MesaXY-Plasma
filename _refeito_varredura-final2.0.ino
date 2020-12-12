//************************************************
//
//			TITULO
//
//***************************************************************
#include <StepDriverCCS.h>

A4988CCS dx, dy; 

unsigned int i; //pq unsigned, ver notas tutorial 4(é unsigned mesmo e nao long)
int led1=12, led2=13, botao=A3; //se funcionar assim, trocar pelo #DEFINE depois
int fcx=9, fcy=10;
int stepx=2, dirx=5, distx=20;
int stepy=3, diry=6, disty=50;
int quant=2, vel=700; //quantidade de varreduras e velocidade de varredura      
float tam=2*25.4; // tam (em polegadas) essa nao trocar pelo #DEFINE (mas trocar pelo const)
				//pois vai ser mudada pelo usuario quando colocar os varios tam assim q o 
				//usuario por, fazer tam=tam*25.4
				
float larg=10; // (em mm) largura do feixe de plasma 
bool sent=HIGH; //HIGH eixo cresce, LOW eixo diminui. 
//(bool ta certo mesmo HIGH nao sendo bool, internamente HIGH é 1 então pode usar bool e fica melhor
//pq podemos fazer sent=!sent).NAO ALTERAR A MENOS QUE A CONFIGURAÇÃO FÍSICA DOS MOTORES MUDAR

void setup() 
{
  dx.pinConfig(stepx,dirx);
  dy.pinConfig(stepy,diry);
  //Define quantos passos o motor realiza por volta
  dx.stepPerRound(200);
  dy.stepPerRound(200);
  
  //Define quantos passos o motor realiza por milimetro
  dx.stepPerMm(200);
  dy.stepPerMm(200); 

  dx.endstopConfig(fcx);
  dy.endstopConfig(fcy);

  pinMode(botao,INPUT);  //botao de comando
  pinMode(led1,OUTPUT);  //leds de indicaçao (no lugar do monitor serial)
  pinMode(led2,OUTPUT);
 
  Serial.begin(9600);

  Serial.println("delay");
  delay(3000);
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
 fc-inicio(); 

}

void loop() 
{  
  //********etapa1: iniciar varredura*********
   Serial.println("ligue o plasma e aperte o botao");
   leds("ligaPlasma"); //led1: pisca, led2: apagado, até botao ser apertado
   
   //********etapa2: varedura iniciada*********
   Serial.println("varredura iniciada."); //led1: aceso, led2: apagado, até a varedura acabar
   leds("iniciada");
   varredura();
   digitalWrite(led1,LOW); //Apaga ambos os leds
   digitalWrite(led2,LOW); //
  
   //***VARREDURA FALTA COLOCAR NA DIAGONAL***
  Serial.println("varredura finalizada, coloque nova amostra.");  
}

void fc-inicio()
{
  //por enquanto vai ter q ir ate o FC de um eixo depois do outro
  //por causa da motormove estar declarada como dx. ou dy. Criar 
  //depois a funçao movimenta pra ser passado quais eixos queremos
 //mover e os valores de cada eixo, assim o movimento pode ser na diagonal
 
 dx.motorMove(200,!sent,697); //200 é um valor qualquer maior do q a mesa(157mm)
 dy.motorMove(200,!sent,697); //pra garantir q ela corra até os FC's

 // vai até a posição "inicio"
 dx.motorMove(78.5-tam/2,sent,700); 
 // nao tem dy. pois defini q o Y do fc-inicio() é o proprio fcy
 
}

//Função responsavel por acionar os leds durante cada etapa
void leds(char etapa[])
{  
  if(etapa=="ligaPlasma") //esperando ligar plasma
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
   
  }else if(etapa=="iniciada") //varredura iniciada
  {
   //led1: aceso, led2: apagado
     digitalWrite(led1,HIGH);
     digitalWrite(led2,LOW);
  
  }
}

//Fica verificando o botao e só para pra trocar o estado 
//dos leds quando o tempo de piscar (300ms) acabar
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

void varredura () //talvez feixe nao precise ser float(so se variar o gas pois isso pode gerar nao inteiro)
{
 const int n=2*((tam/larg)-1)+1;
 //n=numero de passos,precisa ser int pq é oq vai rodar no for.quando declara como int ele
 // pega so a parte inteira(arredonda pra baixo), entao somar 1 garante q ele vai dar um passo a mais, ou seja,
 // arredonadar pra cima e cobrir a placa toda                             
  dy.motorMove((78.5+tam/2),sent,vel);   // isso é a distancia em y da "inicio"
  delay(100);                                // ate o centro da placa + metade da placa. 
  dy.motorMove(tam,!sent,vel); //volta pro canto superior esquerdo da placa

  //a logica usada foi q o for interno so faz uma varredura(de um lado a outro da placa)
 // ai se vezes for par ele vai e volta se for impar so vai.Ai so precisa ficar trocando
 //a direcao do X, por isso o sentido=!sentido no segundo for. Note q "sentido" muda mas "sent" nao
  bool sentido=sent;
  for (int j=0;j<quant;j++) // responsavel por ir e voltar uma quantidade "vezes"
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
   if (quant%2) //vezes é impar
   { 
	 movimenta(tam,(78.5-tam/2),!sent,!sent,696);
	 //movimenta(tamx,tamy,sentx,senty,velo);
   }
   else  //vezes é par
   { dy.motorMove((78.5-tam/2),!sent,696); } //sobe ate a posicao "inicio"
   
  //NOTA: essa funçao é provisoria pois é so pra gnt testar quantas passadas sao necessarias
  //pra de fato deixar homogeneo. 
}

void movimenta(float tamanhoX,float tamanhoY,bool sentX,bool sentY,int velo)
{
	const int n_passos=10;
	//const int n_passosY=10;
	
	float passoX=tamanhoX/n_passos; //quantidadde de passos para o eixo X
	float passoy=tamanhoY/n_passos; //quantidadde de passos para o eixo Y
	
	for (i=0;i<n_passos;i++)
	{
	   dx.motorMove(passoX,sentX,velX);
	   dy.motorMove(passoY,sentY,velY);
	}
}

















