/*========================================================================
    CLASSE GENERICA CLista
	Sinopse: Implementa a Classe Generica Lista encadeada (por ponteiros)
    Autor: Prof. Milton Sakude
			Divisao de Ciencia da Computacao
	Data: 03/04/2003
    Copyright: Qualquer um pode copia-lo e usa-lo livremente sem fins lucrativos..
			   O uso em qualquer software comercial so eh permitido atraves da
			   permissao do autor
==========================================================================*/
#ifndef __CLISTA__
#define __CLISTA__
#include <stdio.h>
#include <stdlib.h>

// Classe generica lista encadeada
// Tipo T eh generico
// Uso: Exemplo para criar uma classe lista de inteiro denominada ClistInt
// typedef	CList<int> CListaInt
//
template <class T> class CLista
{
    // Por convencao se um ponteiro contem o endereco de um no, ele aponta para o proximo
    //	   ________  _______
    //  P->| A |  |->| B |  |->             P aponta para B
    //     --------  --------
private:
    T info; // atributo info (generico))
    CLista<T> *prox; // ponteiro para o proximo nodo
public:
    CLista(); // construtor, inicializa alocando um nodo vazio


    bool EhVazia(); // testa se a lista eh vazia
    bool EhFimLista(); // testa se um ponteiro aponta
    T ObterInfo();	// obtem a informacao info
    void Inserir(T infor); // insere na posicao apontada por P, P aponta para o elemnto inserido
    // Exemplo L={A,B,C}, se P aponta para B, P->Inserir(X), L={A,X,B,C}
    void InserirFim(T infor);  // Insere no fim da lista
    void InserirOrd(T infor, bool Compara(T info1,T info2));
    // Inserir ordenado
    // Compara deve ser uma funcao a ser elaborada e passada como parametro
    // Serve para ordenar segundo a necessidade, de acordo com uma chave por exemplo
    // Suponha que um tipo T tenha um atributo chave
    // Exemplo bool ComparaMenor(T info1, T info2)
    //				{ return info1.chave>info2.chave;}
    // Ordena em ordem crescente

    void Remover();	// Remove o elemento apontado por P e retorna o proximo ponteiro
    // OBSERVACAO: O uso geral deve ser precedido da chama da funcao Localizar
    CLista<T> *Localizar(T infor,bool Compara(T info1,T info2)); // procura uma informacao de acordo a a funcao Compara
    CLista<T> *Achar(T infor,bool Compara(T info1,T info2)); // procura uma informacao de acordo a a funcao Compara
    // Vide exemplo anterior. Naquele caso, encontra a posicao do elemento imediatamenta maior
    // Exemplo L={1,2,3,5}infor=4, retorna o ponteiro para 5
    // No caso do codigo ser 	{ return info1.chave==info2.chave;}
    // localiza o elemento procurado
    CLista<T> *Proximo();  // avanca o ponteiro para o proximo elemento
    CLista<T> *FimLista(CLista *list_ptr); // avanca o ponteiro para o Fim da lista
    // NAO CONFUNDA COM EhFimLista
    void Mostrar(void ImprimirInfo(T info)); // Lista todos os elementos a partir do apontamento
    // Deve-se elaborar a funcao ImprimirInfo para imprimir um elemento
    void Gravar(char *arquivo,void GravarInfo(FILE *file,T info));
    // Grava lista em um arquivo.
    // A funcao GravarInfo(FILE *file,T info)) deve ser elaborada para gravar cada elemento
    void Ler(char *arquivo, bool LerInfo(FILE *file, T *info));
    // Ler lista de um arquivo.
    //A funcao GravarInfo(FILE *file,T info)) deve ser elaborada para ler cada elemento
    void Processa(void ProcessaInfo(T info)); // Processa todos os elementos a partir do apontamento
    // Deve-se elaborar a funcao ProcessaInfo para fazer alguma processamento com um elemento

//     void Destruir();// destrutor, libera memoria dos nodos alocados

};

// Implementacao dos metodos definidas pela interface
// Construtor: inicializa alocando um nodo vazio
template <class T>
CLista<T>::CLista()
{
    prox=NULL;
}

// Destroi lista, libera memoria dos nodos alocados
/*template <class T>
void CLista<T>::Destruir()
{  CLista *Temp=this;
    while (Temp->prox!=NULL)
       Temp->Remover();
    delete Temp;
}
*/
// testa se a lista eh vazia
template <class T>
bool CLista<T>::EhVazia()
{
    return (prox==NULL);
}

// obtem a informacao info
template <class T>
T CLista<T>::ObterInfo()
{
    return prox->info;
}

// Testa se fim de lista
template <class T>
bool CLista<T>::EhFimLista()
{
    return (prox==NULL);
}

// insere na posicao apontada por P, P aponta para o elemnto inserido
template <class T>
void CLista<T>::Inserir(T infor)
{
    CLista *list_ptr= new CLista;
    list_ptr->prox=prox;
    prox=list_ptr;
    list_ptr->info=infor;
}


// Insere no fim da lista
template <class T>
void CLista<T>::InserirFim(T infor)
{
    CLista *list_ptr=this;
    while (list_ptr->prox!=NULL) {
        list_ptr=list_ptr->prox;
    }
    list_ptr->Inserir(infor);
}

// avanca o ponteiro para o proximo elemento
template <class T>
CLista<T> *CLista<T>::Proximo()
{
    CLista *p=this;
    if (p->prox) {
        return p->prox;
    }
    return NULL;
}

// Avanca para o fim da lista
template <class T>
CLista<T>* CLista<T>::FimLista(CLista *lista)
{
    CLista<T> *lista_ptr=lista;
    while (lista_ptr->prox!=NULL) {
        lista_ptr=lista_ptr->prox;
    }
    return lista_ptr;
}

// Localiza a informacao segundo a funcao Compara, aponta para NULL se não achou
template <class T>
CLista<T> *CLista<T>::Localizar(T infor,bool Compara(T info1,T info2))
{
    CLista<T> *list_ptr=this;
    while (list_ptr->prox!=NULL &&(!Compara(list_ptr->ObterInfo(),infor))) {
        list_ptr=list_ptr->prox;
    }
    if (!EhFimLista()) {
        return list_ptr;
    }
    return NULL;
}
// Achar a informacao segundo a funcao Compara, aponta para o fim da lista se não achou
template <class T>
CLista<T> *CLista<T>::Achar(T infor,bool Compara(T info1,T info2))
{
    CLista<T> *list_ptr=this;
    while (list_ptr->prox!=NULL &&(!Compara(list_ptr->ObterInfo(),infor))) {
        list_ptr=list_ptr->prox;
    }

    return list_ptr;

}
// Insere ordenado com efeitos de acordo com a funcao Compara
template <class T>
void CLista<T>::InserirOrd(T infor, bool Compara(T info1,T info2))
{
    CLista<T> *list_ptr=Achar(infor,Compara);
    list_ptr->Inserir(infor);
}

// Remove o elemento a qual eh apontado
template <class T>
void CLista<T>::Remover()
{
    CLista<T> *list_ptr=this;
    if (list_ptr) {
        CLista *temp=list_ptr->prox;
        list_ptr->prox=temp->prox;
        delete temp;
    }

}


// Mostrar a lista na tela
template <class T>
void CLista<T>::Mostrar(void ImprimirInfo(T info))
{
    CLista<T> *list_ptr=this;
    T info;
    while (list_ptr->prox!=NULL) {
        info=list_ptr->ObterInfo();
        ImprimirInfo(info);
        list_ptr=list_ptr->prox;
    }
    printf("\n");
}

// Gravar a lista em um arquivo
template <class T>
void CLista<T>::Gravar(char *arquivo, void GravarInfo(FILE *file,T info))
{
    FILE *file=fopen(arquivo,"w");
    CLista<T> *list_ptr=this;
    T info;
    while (list_ptr->prox!=NULL) {
        info=list_ptr->ObterInfo();
        GravarInfo(file,info);
        list_ptr=list_ptr->prox;
    }
    fclose(file);
}

// Ler lista de um arquivo
template <class T>
void CLista<T>::Ler(char *arquivo, bool LerInfo(FILE *file,T *info))
{
    FILE *file=fopen(arquivo,"r");
    T info;
    CLista<T> *list_ptr=this;
    if (file)
        while (!feof(file)) {
            if (LerInfo(file,&info)) {
                list_ptr->Inserir(info);
                list_ptr=list_ptr->prox;
            }
        }
    fclose(file);
}

// Processa uma lista segundo o que faz Processa Info
template <class T>
void CLista<T>::Processa(void ProcessaInfo(T info))
{
    CLista<T> *list_ptr=this;
    T info;
    while (list_ptr->prox!=NULL) {
        info=list_ptr->ObterInfo();
        ProcessaInfo(info);
        list_ptr=list_ptr->prox;
    }

}

#endif
