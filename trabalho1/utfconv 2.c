 /* Leonardo Edelman Wajnsztok 1312737 3WB */
 /* Jordana Mecler 1311354 3WB */
 
 #include <stdio.h>
 #include <stdlib.h>
 #include "utfconv.h"
 
int testa_bom (unsigned char *bom)
{
	if(bom[0]==0xFF && bom[1] == 0xFE && bom[2] == 0x00 && bom[3] ==0x00) //Little Endian
		return 1;
	else if(bom[3]==0xFF && bom[2] == 0xFE && bom[1] == 0x00 && bom[0] ==0x00)  //Big Endian
		return 0;
	else                 //Erro                                                       
		return -1;
}

int verifica_caracter_8(unsigned char *x, int n)
{
	unsigned char temp1,temp2, temp3;  //precisa das temporarias pra nao modificar o conteudo real!
	if(n==2)
	{
		temp1 = x[1]>>6;         //testa se  os bytes tem o padrao 0x10******
		if(temp1==0x2) return 1;
		return -1;
	}
	else if(n==3)
	{
		temp1 =x[1]>>6;
		temp2 =x[2]>>6;
		if(temp1==0x2 && temp2==0x2) return 1;
		return -2;		
	}
	else
	{
		temp1 =x[1]>>6;
		temp2 =x[2]>>6;
		temp3 =x[3]>>6;
		if(temp1==0x2 && temp2==0x2 && temp3==0x2) return 1;
		return -3;	
	}
}

int verifica_tamanho (unsigned char x)
{
    x=x>>4;
    if(x==0xF)
        return 4;
    else if(x==0xE)            //devolve o tamanho de bytes que serao gravados ou retorna erro se nao estiver no padrao
        return 3;
    else if(x==0xC || x==0xD)
        return 2;
    else if(x<=0x7)
        return 1;
    else return -1;
}

void conversao8_32 (unsigned char *x, int n)
{
    unsigned char temp;
    if(n==2) 
    {
        temp=x[0]<<6;
        x[1]&=0x3F;
        x[1]|=temp;
        x[0]>>=2;
        x[0]&=0x07;
        x[3]=x[1];
        x[2]=x[0];
        x[1]=x[0]=0;
    }
    else if(n==3)
    {
        temp=x[1]<<6;
        x[2]&=0x3F;
        x[2]|=temp;
        temp=x[0]<<4;
        x[1]&=0x3F;
        x[1]>>=2;
        x[1]|=temp;
        x[3]=x[2];
        x[2]=x[1];
        x[1]=x[0]=0;
    }
    else
    {
        temp=x[2]<<6;
        x[3]&=0x3F;
        x[3]|=temp;
        temp=x[1]<<4;
        x[2]&=0x3F;
        x[2]>>=2;
        x[2]|=temp;
        temp=x[0]<<4;
        x[1]&=0x3F;
        x[1]>>=4;
        x[1]|=temp;
        x[0]=0;
    }
}

void inverte_vetor (unsigned char *x) // Inverte os bytes para o caso de ser Little Endian
{
	unsigned char k;
	k=x[0];
	x[0]=x[3];
	x[3]=k;
	k=x[1];
	x[1]=x[2];
	x[2]=k;
}

 int conversao32_8 (unsigned char *x)
{
    if(x[0]==0 && x[1]==0 && x[2]==0 && x[3]<=0x7F)  // primeiro caso onde só devolve 1 byte e nao altera
    {
    	return 1; //1 byte
    }
    else if(x[0]==0 && x[1]==0 && x[2]<=0x07)   // segundo caso de conversão32_8
    {
	unsigned char temp;
	x[2]<<=2;
	temp=x[3]>>6;
	x[2]|=temp;
	x[2]|=0xC0;
	x[3]|=0x80;
	x[3]&=0xBF;
        return 2;
    }
    else if (x[0]==0 && x[1]==0 && x[2]<=0xFF)  // terceiro caso de conversão32_8
    {
        unsigned char temp, temp2;
	temp=x[2]>>4;
	x[1]=x[1]|temp;
	temp=x[3]>>6;
	temp2=x[2]<<2;
	x[2]=temp|temp2;
	x[1]|=0xE0;
	x[2]|=0x80;
	x[3]|=0x80;
	x[2]&=0xBF;
	x[3]&=0xBF;
        return 3;
    }
    else    // quarto caso de conversão32_8
    {
        unsigned char temp1, temp2;
	temp1=x[1]>>2;
	x[0]=temp1;
	temp1=x[2]>>4;
	temp2=x[1]<<4;
	x[1]=temp1|temp2;
	temp1=x[2]<<2;
	temp2=x[3]>>6;
	x[2]=temp1|temp2;
	x[0]|=0xF0;
	x[1]|=0x80;
	x[1]&=0xBF;
	x[2]|=0x80;
	x[2]&=0xBF;
	x[3]|=0x80;
	x[3]&=0xBF;
        return 4;
    }
}
 
int conv8_32(FILE *arq_entrada, FILE *arq_saida, char ordem)
{
	int n,lidos;
    unsigned char conteudo[4];
    unsigned int pos=0;
    if(ordem=='B')
    {
        conteudo[0]=conteudo[1]=0x00;   //big endian
        conteudo[2]=0xFE;
        conteudo[3]=0xFF;
    }
    else if(ordem=='L')                 //little endian
    {
        conteudo[0]=0xFF;
        conteudo[1]=0xFE;
        conteudo[3]=conteudo[2]=0x00;
    }
    else
    {
    	fprintf(stderr,"Ordem %c nao existe! Escolha entre B e L.\n",ordem);
      			return -1;
    }
    fwrite(conteudo,sizeof(char),4,arq_saida);							//grava o bom
    while((lidos=fread(conteudo,sizeof(char),1,arq_entrada))==1)	//le primeiro byte
    {
        pos+=lidos;
        n=verifica_tamanho(conteudo[0]);					//verifica se precisa ler mais bytes e retorna a quantidade
        if(n==1)
    	{   
        	conteudo[3]=conteudo[0];
            conteudo[0]=conteudo[1]=conteudo[2]=0;
        }
        else
        {      
        	if(n==-1)
        	{
        		fprintf(stderr,"Caracter invalido! Byte na posição %d fora do padrao UNICODE.\n",pos);
      			return -1;
			}  
            if((lidos=fread(&conteudo[1],sizeof(char),n-1,arq_entrada))!=n-1)   // verifica se houve termino do arquivo inesperado
            {
            	fprintf(stderr,"Final de arquivo inesperado.\n");
        		return -1;
            }
            pos+=lidos;
            if(verifica_caracter_8(conteudo,n)!=1)
            {
            	fprintf(stderr,"Caracter invalido! Byte na posição %d fora do padrao UNICODE.\n",pos);
      			return -1;
            }
            conversao8_32(conteudo,n);
        }
        if(ordem == 'L')
        	inverte_vetor(conteudo);
        fwrite(conteudo,sizeof(char),4,arq_saida);
    }
    return 0;
}

int conv32_8(FILE *arq_entrada, FILE *arq_saida)
{
  int n, lidos,bom;             	       // byte order mark
  unsigned int pos=0;
  unsigned char conteudo[4];
 		
  pos+=fread(conteudo,sizeof(char),4,arq_entrada);
  bom=testa_bom(conteudo);
    if(bom==1)
    {
     	while((lidos=fread (conteudo,sizeof(char),4,arq_entrada))==4)   //Enquanto ler 4 bytes, armazena no vetor de char
  	    {
  	    	pos+=lidos;
	       inverte_vetor(conteudo); // Inverte os bytes porque é Litte Endian
	       if(conteudo[1]>0x10)
  	    	{
  	    		fprintf(stderr,"Caracter invalido! Byte na posicao %d fora do padrao UNICODE.\n",pos-1);
  	    		return -1;
  	    	}
  		    n=conversao32_8(conteudo);
		    fwrite(&conteudo[4-n], 1, n, arq_saida);
  		  	
        }
    }
    else if(bom==0) // Big Endian
    {
        while((lidos=fread (conteudo,sizeof(char),4,arq_entrada))==4)  //Enquanto ler 4 bytes, armazena no vetor de char
  	    {
  	    	pos+=lidos;
  	 		if(conteudo[1]>0x10)
  	    	{
  	    		fprintf(stderr,"Caracter invalido! Byte na posicao %d fora do padrao UNICODE.\n",pos-2);
  	    		return -1;
  	    	}
  		    n=conversao32_8(conteudo);
		    fwrite(&conteudo[4-n], 1, n, arq_saida);
		}
    }
    else
    {
    	fprintf(stderr,"BOM inválido.\n");
    	return -1;
	}
    if(lidos!=0)    // se houve sobra de bytes (de 1 a 3)
    {
        fprintf(stderr,"Final de arquivo inesperado.\n");
        return -1;
    }
  return 0;
}
