/*
 * jsmnRipper.h
 *
 *  Created on: 21 may. 2018
 *      Author: ipserc
 */

#ifndef SOURCE_JSMNRIPPER_H_
#define SOURCE_JSMNRIPPER_H_

#include </opt/include/list.h>
#include </opt/include/jsmn.h>
#include </opt/include/errtra.h>

#define VERSION "1.2"
#define COMPILATION "2018-06-02"

/**
 * Item structure which stores the string path token in a linked list-
 * name is the token name which indicates a field of the JSON message
 * jtype is the JSMN type of the token (JSMN_OBJECT or JSMN_ARRAY)
 * index is the index of the field if the type is JSMN_ARRAY
 */
typedef struct
{
	char * name;
	jsmntype_t jtype;
	int index; //If array, the index of the item
} item_t;

/*
 * Prototypes
 */
/* jsmnRipper.c */
int getJsmnTokenLen(jsmntok_t *jsmnToken);
int listTokenCreate(list_t *tokenList, char *tpath);
void printItem(void *param);
void freeItem(item_t *item);
int parseJSON(char *jsonMsg, jsmn_parser *parser, jsmntok_t **jsmnTokenArray);
_Bool nextToken(jsmntok_t **jsmnToken);
_Bool prevToken(jsmntok_t **jsmnToken);
_Bool lastToken(jsmntok_t *jsmnToken);
_Bool jumpToTokenPos(jsmntok_t **jsonToken, int newPosition);
void printJsmnToken(char *jsonMsg, jsmntok_t *jsmnToken);
void printJsmnTokens(char *jsonMsg, jsmntok_t *jsmnTokenArray);
void printJsmnTokenValue(char *jsonMsg, jsmntok_t *jsmnToken);
jsmntok_t *findJsmnEngine(list_t *tokenList, char *jsonMsg, jsmntok_t *jsmnTokenArray);
jsmntok_t *findJsmnToken(char *tpath, char *jsonMsg, jsmntok_t *jsmnTokenArray);
char *getTokenValue(char *tpath, char *jsonMsg, jsmntok_t *jsmnTokenArray);

#endif /* SOURCE_JSMNRIPPER_H_ */
