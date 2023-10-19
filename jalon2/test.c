#include <stdio.h>
#include <string.h>
#include <stdlib.h>
void affiche(char** tab){
    int i = 0;
    while(tab[i]!=NULL){
        printf("%s\n",tab[i]);
        i++;
    }
}

int get_length(char** words) {
    int length = 0;
    while (words[length] != NULL) {
        length++;
    }
    return length;
}

char ** split_sentence(char * str, int TAILLE_MAX_OF_WORDS){
    char *token = strtok(str, " ");
    char ** words = (char ** )malloc(sizeof(char*)*TAILLE_MAX_OF_WORDS); // Tokenize the string
    //strcpy(words[0],token);
    int i = 0;
    while (token != NULL) {
        words[i] = token;
        token = strtok(NULL, " ");
        i++;
    }    
    words[i] = NULL;
    return words;
}

char * concatenate(char ** words, int index){
    char * result = (char*)malloc(sizeof(char*) * (get_length(words)-1));
    result[0] = '\0';
    for(int i = index; i < get_length(words); i++)
    {
        strcat(result,words[i]);
        if (i < get_length(words) - 1)
        {
                strcat(result, " ");
        }
    }
    return result;

}

int main() {
    char str[] = "This";
    char ** words = split_sentence(str,20);
    printf("%s\n",concatenate(words,1));
    //affiche(words);

    return 0;
}
