/* Leonardo Edelman Wajnsztok 1312737 3WB
   Jordana Zerpini Mecler 1311354 3WB*/
#include <stdio.h>
#include <stdlib.h>
  
typedef int (*funcp) (int x, int y, int z, int k, int g); 

typedef struct lista
{ 
    int posif;
    int linhagoto;
    struct lista * prox;
} Lista;

void preenche_gen(unsigned char *codigo, int *pos, int n, unsigned char *hex)
{
    int i;
    for (i = 0; i < n; i++,(*pos)++)  /*funcao para preencher n casas do vetor com codigo de assembly*/
        codigo[(*pos)] = hex[i];
}

void preenche_idx (unsigned char *vet,int *pos, int num)
{
    int i;
    for(i=0;i<4;i++,(*pos)++)          /*funcao para preencher casas do vetor com os valores inteiros lidos do minima*/
        vet[(*pos)]=(unsigned char)(num>>(i*8));
}

void preenche_vr(unsigned char *codigo, char var,int *pos, int idx)
{
    if(var=='v')
        codigo[(*pos)++] = 0xfc - 4*idx;    /*funcao para preencher o vetor de acordo com a posicao na pilha*/
    else
        codigo[(*pos)++] = 0x08 + 4*idx;    /*pode ser tanto para var local, quanto para parametro*/
}

Lista *insere(Lista *dic,int num, int pos)
{
    Lista *novo = (Lista*)malloc(sizeof(Lista));
    if(!novo) 
    {
        printf("Falha na aLocacao de memoria.\n");
        exit(0);
    }
    novo->prox = dic;
    novo->posif = pos;
    novo->linhagoto = num;
    return novo;
}

void libera(Lista *dic)
{
    Lista *prox;
    while(dic)
    {
        prox = dic->prox;
        free(dic);
        dic = prox;
    }
}

funcp geracod (FILE *f)
{
    Lista *dic=NULL,*plista;    /*inicio[] contem a incializacao em assembly e criacao de 5 var locais (push %ebp... sub $20,%esp)*/
    int ref[50], pos=0, linha = 0;  /*final[] contem finalizacao ate retorno*/
    char c;                         /*vif[] tem cpml ecx com edx,  je para ff, que dps sera alterado*/
    unsigned char inicio[] = {0x55,0x89,0xe5,0x83,0xec,0x14}, final[] = {0x89,0xec,0x5d,0xc3}, vif[] = {0x39,0xca,0x74,0xff};
    unsigned char *codigo = (unsigned char*)malloc(2048*sizeof(unsigned char));

    preenche_gen(codigo,&pos,6,inicio);

    while ((c=fgetc(f))!=EOF)
    {
        switch (c)
        {
            case 'r':   /*caso retorno*/
            {
                int idx; 
                char var;
              
                ref[linha++] = pos;
                fscanf(f,"et %c%d", &var, &idx);
                if(var=='$')    /*caso constante*/
                {                    
                    codigo[pos++] = 0xb8;  /*movl constante pra eax*/
                    preenche_idx(codigo,&pos,idx);
                }
                else   /*caso parâmetro ou variável local*/
                {
                    codigo[pos++] = 0x8b;  /*movl da pilha pra eax*/
                    codigo[pos++] = 0x45;

                    preenche_vr(codigo,var,&pos,idx);
                }
                preenche_gen(codigo,&pos,4,final);
                break;
            }
            case 'v': case 'p': /*caso atribuição*/
            {
                int idx0, idx1, idx2;
                char var0=c, var1, var2;
                char op;
                ref[linha++] = pos;
                fscanf(f,"%d := %c%d %c %c%d", &idx0, &var1, &idx1, &op, &var2, &idx2);
                if(var1 == '$') /*caso 1a variavel constante*/
                {
                    codigo[pos++] = 0xb9; /*movl constante pro ecx*/
                    preenche_idx(codigo,&pos,idx1);
                }
                else  /*caso 1a variavel parâmetro ou variável local*/
                {
                    codigo[pos++] = 0x8b; /*movl da pilha pro ecx*/
                    codigo[pos++] = 0x4d;
                    preenche_vr(codigo,var1,&pos,idx1);                           
                }

                if(var2 == '$')  /*caso 2a variável constante*/
                {
                    if(op == '+')  /*caso soma*/
                    {
                        codigo[pos++] = 0x81; /*addl const no ecx*/
                        codigo[pos++] = 0xc1;

                    }
                    else if (op == '-')   /*caso subtração*/
                    {
                        codigo[pos++] = 0x81;  /*subl const no ecx*/
                        codigo[pos++] = 0xe9;
                    }
                    else   /*caso multiplicação*/
                    {
                        codigo[pos++] = 0x69;   /*imull const no ecx*/
                        codigo[pos++] = 0xc9;
                    }
                    preenche_idx(codigo,&pos,idx2);
                }
                else  /*caso 2a variável parâmetro ou variável local*/
                {
                    if (op == '+')
                    {
                        codigo[pos++] = 0x03; /*addl pilha no ecx*/
                        codigo[pos++] = 0x4d;
                    }
                    else if(op == '-')
                    {
                        codigo[pos++] = 0x2b;   /*subl pilha no ecx*/
                        codigo[pos++] = 0x4d;
                    }
                    else
                    {
                        codigo[pos++] = 0x0f;   /*imull pilha no ecx*/
                        codigo[pos++] = 0xaf;
                        codigo[pos++] = 0x4d;
                    }
                    preenche_vr(codigo,var2,&pos,idx2);
                }
                codigo[pos++] = 0x89;   /*movl do ecx pra pilha*/
                codigo[pos++] = 0x4d;
                preenche_vr(codigo,var0,&pos,idx0);
                break;
            }
            case 'i':   /*caso if goto*/
            {
                int idx0, idx1, num;
                char var0, var1;
                ref[linha++] = pos;
                fscanf(f,"feq %c%d %c%d %d", &var0, &idx0, &var1, &idx1, &num);
                if(var0 == '$')   /*caso 1a variável constante*/
                {
                    codigo[pos++] = 0xb9; /*movl const pro ecx*/
                    preenche_idx(codigo,&pos,idx0);
                }
                else   /*caso 1a variável parâmetro ou variável local*/
                {
                    codigo[pos++] = 0x8b; /*movl pilha pro ecx*/
                    codigo[pos++] = 0x4d;
                    preenche_vr(codigo,var0,&pos,idx0); 
                }
                if (var1 == '$')   /*caso 2a variável constante*/
                {
                    codigo[pos++] = 0xba;  /*movl const pro edx*/
                    preenche_idx(codigo,&pos,idx1);
                }
                else   /*caso 2a variável parâmetro ou variável local*/
                {
                    codigo[pos++] = 0x8b; /*movl pilha pro edx*/
                    codigo[pos++] = 0x55; 
                    preenche_vr(codigo,var1,&pos,idx1);
                }
                preenche_gen(codigo,&pos,4,vif);
                dic = insere(dic,num,pos-1);
                break;
            }
        }
        fscanf(f," ");
    }
    plista = dic;
    while(plista)  /*preenchimento dos bytes do vetor de código de máquina que ficaram com valor qualquer por meio do dicionário de realocação (linha do goto)*/
    {
        int poscodigo = ref[(plista->linhagoto)-1];
        char off = (int) &codigo[poscodigo] - (int)&codigo[(plista->posif)+1];    
        codigo[plista->posif] = off;
        plista = plista->prox;        
    }
    libera(dic);
    return (funcp)codigo;
} 