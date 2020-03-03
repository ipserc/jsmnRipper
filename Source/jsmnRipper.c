/*
 * jmsnRipper.c
 *
 *  Created on: 21 may. 2018
 *      Author: ipserc
 */

#include "jsmnRipper.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define JSMN_PARENT_LINKS

/**
 * Structural Function. Gets the length of the token pointed by jsmnToken.
 * @param jsmnToken jsmntok_t*
 * @return Token length
 */
int getJsmnTokenLen(jsmntok_t * jsmnToken)
{
	return jsmnToken->end - jsmnToken->start;
}

/**
 * Structural Function. Transforms the token path from string format to an item of item_t type.
 * Appends the item found in linked list of items.
 * @param tokenList list_t*
 * @param tpath char* token path
 * @return the number of tokens in the list
 */
int listTokenCreate(list_t * tokenList, char * tpath)
{
	item_t item;
	char * strIndex;
	char * tpathtok = strtok(tpath, ".");

	while (tpathtok)
	{
		item.index = 0;
		item.jtype = JSMN_OBJECT;
		strIndex = strchr(tpathtok, '[');
		if (strIndex)
		{
			*strIndex++ = '\0';
			item.jtype = JSMN_ARRAY;
			while (*strIndex != ']')
			{
				item.index *= 10;
				item.index += *strIndex++ - '0';
			}
		}

		item.name = malloc(strlen(tpathtok)+1);
		sprintf(item.name, "%s", tpathtok);
		//TRACE("Allocated item->name(%p):%s",item.name, item.name);
		tokenList = listAppend(tokenList, &item, sizeof(item_t));
		tpathtok = strtok(NULL, ".");
	}
	return (int)listItemsCount(tokenList);
}

/**
 * Structural Function. Prints the info stored in the item.
 * It is useful for debugging purposes
 * @param param void* an item
*/
void printItem(void * param)
{
	item_t * item  = (item_t *)param;
	//TRACE("Address item(%p)",item);
	//TRACE("Address item->name(%p):%s",item->name, item->name);
	printf("item->name:%s\n", item->name);
	printf("item->jtype:%d\n", item->jtype);
	printf("item->index:%d\n", item->index);
}

/**
 * Structural Function. Frees the allocated memory for "name" in the item_t structure
 * @param item item_t*
 */
void freeItem(item_t * item)
{
	//item = (list_t *)itemList->tail->item
	//TRACE("freeing item->name(%p):%s", item->name, item->name);
	free(item->name);
}

/**
 * Main call to JSMN parser. A previous call to this function is required to use findJsonToken and getTokenValue.
 * jsmn_parse parses the JSON message and put it into an array of tokens.
 * jsmnTokenArray points the beginning of the array.
 * The last item of the array is a "0" token, so we can control when we have reached the end of the array if we get a "0" of jsmntok_t type.
 * Once you have finished using the jsmnTokenArray array, you have to free it.
 * @param jsonMsg char*
 * @param parser jsmn_parser*
 * @param jsmnTokenArray jsmntok_t**
 * @return tokenCount int number of tokens
 */
int parseJSON(char * jsonMsg, jsmn_parser * parser, jsmntok_t ** jsmnTokenArray)
{
	jsmn_init(parser);
	int tokenCount = jsmn_parse(parser, jsonMsg, strlen(jsonMsg), (jsmntok_t *)NULL, 0);
	if (tokenCount < 0) {
		  printf("unable to read tokens from JSON message");
		  return tokenCount;
	}
	// one token more to end the array with nulls
	*jsmnTokenArray = malloc(sizeof(jsmntok_t) * (tokenCount+1));
	memset(*jsmnTokenArray, 0, sizeof(jsmntok_t) * (tokenCount+1));
	jsmn_init(parser);
	jsmn_parse(parser, jsonMsg, strlen(jsonMsg), *jsmnTokenArray, sizeof(jsmntok_t) * (tokenCount+1));
	return tokenCount;
}

/**
 * Structural Function. Jumps to the next token pointed by jsmnToken
 * Returns false in case next token were the null token which marks the end of the array, otherwise, it returns true.
 * If jsmnToken is the NULL token, no jump to the next token is done. The pointer remains at the NULL token.
 * @param jsmnToken jsmntok_t**
 * @return false (0) if there aren't any more tokens, true (any number) if there are one more
 */
bool nextToken(jsmntok_t ** jsmnToken)
{
	jsmntok_t * token = *jsmnToken;
	if (!(token->start + token->end + token->size + token->parent)) return false;
	++(*jsmnToken);
	token = *jsmnToken;
	return (token->start + token->end + token->size + token->parent);
}
/**
 * Structural Function. Jumps to the previous token pointed by jsmnToken
 * Returns false in case previous token were outside of the start of the array
 * Otherwise, returns true.
 * @param jsmnToken jsmntok_t**
 * @return false if there aren't a previous token, true if there are a previous one
 */
bool prevToken(jsmntok_t ** jsmnToken)
{
	jsmntok_t * token = *jsmnToken;
	if (--token->start == 0) return false;
	--(*jsmnToken);
	return true;
}

/**
 * Structural Function. Checks if the token pointed by jsonToken is the last token of the tokens array.
 * Returns TRUE if the token pointed by jsonToken is the last.
 * @param jsmnToken jsmntok_t**
 * @return true if is the last token, false if not
 */
bool lastToken(jsmntok_t * jsmnToken)
{
	return !(jsmnToken->start + jsmnToken->end + jsmnToken->size + jsmnToken->parent);
}

/**
 * Structural Function. Jumps to the token which starts at the next position given by newPosition.
 * @param jsonToken jsmntok_t**
 * @param newPosition int
 * @return false (0) if there aren't any more tokens, true (any number) if there are one more
 */
bool jumpToTokenPos(jsmntok_t ** jsonToken, int newPosition)
{
	bool retVal = false;
	while (((jsmntok_t *)(*jsonToken))->start < newPosition) retVal = nextToken(jsonToken);
	return retVal;
}

/**
 * Structural Function. Prints the token value and the start, end, size, type, and parent values of an specific token pointed by jsonToken
 * @param jsonMsg char*
 * @param jsmnToken jsmntok_t*
 */
void printJsmnToken(char * jsonMsg, jsmntok_t * jsmnToken)
{
	char tokenfmt[80];

	sprintf(tokenfmt, "%s%lu%s", "Token:%.", (size_t)getJsmnTokenLen(jsmnToken), "s start:%d end:%d size:%d type:%d parent:%d\n");
	printf(tokenfmt, &jsonMsg[jsmnToken->start], jsmnToken->start, jsmnToken->end, jsmnToken->size, jsmnToken->type, jsmnToken->parent);
}

/**
 * Structural Function. Prints the information of all the tokens of the parsed JSON.
 * @param jsonMsg char*
 * @param jsmnTokenArray jsmntok_t*
 */
void printJsmnTokens(char * jsonMsg, jsmntok_t * jsmnTokenArray)
{
	jsmntok_t * jsmnToken = jsmnTokenArray;

	while (nextToken(&jsmnToken))
	{
		printJsmnToken(jsonMsg, jsmnToken);
	}
}

/**
 * Structural Function. For an specific token prints the value store in its next token.
 * @param jsonMsg char*
 * @param jsmnToken jsmntok_t*
 */
void printJsmnTokenValue(char * jsonMsg, jsmntok_t * jsmnToken)
{
	printJsmnToken(jsonMsg, ++jsmnToken);
}

/**
 * Structural Function. Finds the token mapped in the path kept in the list tokenList.
 * Returns the token found or NULL if the token couldn't be found or doesn't exist.
 * @param tokenList list_t*
 * @param jsonMsg char*
 * @param jsmnTokenArray jsmntok_t*
 * @return the token found or NULL if the token couldn't be found or doesn't exist
 */
jsmntok_t * findJsmnEngine(list_t * tokenList, char * jsonMsg, jsmntok_t * jsmnTokenArray)
{
	jsmntok_t * jsmnToken = jsmnTokenArray;
	node_t * tokenListNode = tokenList->head;
	item_t * item = (item_t *)((node_t *)(tokenListNode->item));
	char * jsmnTokenItem;
	int indexCount = 0;
	int tokenIndex = 0;
	bool cont; //Continue flag

	if (getJsmnTokenLen(jsmnToken) < 0) return (jsmntok_t *) NULL;
	if (!memcmp("", &jsonMsg[jsmnToken->start], (size_t)getJsmnTokenLen(jsmnToken))) return (jsmntok_t *) NULL;
	cont = nextToken(&jsmnToken);
	while (cont)
	{
		jsmnTokenItem = malloc((size_t)getJsmnTokenLen(jsmnToken)+1);
		jsmnTokenItem[(size_t)getJsmnTokenLen(jsmnToken)] = '\0';
		strncpy(jsmnTokenItem, &jsonMsg[jsmnToken->start], (size_t)getJsmnTokenLen(jsmnToken));
		//TRACE("Token to find item->name:%s", item->name);
		//TRACE("jsmnTokenItem:%s", jsmnTokenItem);
		//printf("%s", "***** Test "); printJsmnToken(jsonMsg, jsmnToken); puts("");

		if (!strcmp(item->name, jsmnTokenItem))
		{
			if (item->jtype == JSMN_ARRAY)
			{
				tokenIndex = item->index;
			}

			cont = nextToken(&jsmnToken);
			if (jsmnToken->type == JSMN_ARRAY)
			{
				cont = nextToken(&jsmnToken);
			}
			if (jsmnToken->type == JSMN_OBJECT)
			{
				while (tokenIndex > indexCount)
				{
					++indexCount;
					cont = jumpToTokenPos(&jsmnToken, jsmnToken->end);
				}
				cont = nextToken(&jsmnToken);
			}
			if (tokenListNode->next)
			{
				indexCount = 0;
				tokenIndex = 0;
				tokenListNode = tokenListNode->next;
				item = (item_t *)((node_t *)(tokenListNode->item));
				//TRACE("**** %s Token to find item->name:%s", (tokenListNode->next) ? "Next" : "Last", item->name);
			}
			else // item found
			{
				free(jsmnTokenItem);
				return jsmnToken;
			}
		}
		else
		{
			switch (jsmnToken->type)
			{
				case JSMN_OBJECT:
					cont = jumpToTokenPos(&jsmnToken, jsmnToken->end);
					break;
				case JSMN_ARRAY:
				case JSMN_STRING:
				case JSMN_PRIMITIVE:
				case JSMN_UNDEFINED:
					cont = nextToken(&jsmnToken);
					break;
			}
		}
		free(jsmnTokenItem);
	}
	return (jsmntok_t *)NULL;
}

/**
 * Finds the token mapped in the string path.
 * Path should be expressed in the form "field1.field2[index2].field3[index3]. ... .fieldN"
 * Returns the token found or NULL if the token doesn't exist.
 * @param tpath char*
 * @param jsonMsg char*
 * @param jsmnTokenArray jsmntok_t*
 * @return jsmntok_t* the token found or NULL if not found
 */
jsmntok_t * findJsmnToken(char * tpath, char * jsonMsg, jsmntok_t * jsmnTokenArray)
{
	list_t * tokenList;
	jsmntok_t * jsmnTokenFound = (jsmntok_t *)NULL;
	char * tokenPath = malloc(strlen(tpath)+1);

	sprintf(tokenPath, "%s", tpath);
	listNew(&tokenList);
	//TRACE("tokenPath:%s:", tokenPath);
	if (listTokenCreate(tokenList, tokenPath))
	{
		jsmnTokenFound = findJsmnEngine(tokenList, jsonMsg, jsmnTokenArray);
		listDestroy(tokenList, (void *)freeItem);
	}
	//else WARNING("%s", "Unable to generate token path list.");
	free(tokenPath);
	return jsmnTokenFound;
}

/**
 * Finds the token mapped in the string path.
 * Path should be expressed in the form "field1.field2[index2].field3[index3]. ... .fieldN"
 * Returns the VALUE token found which is the next token in the sequence of the tokens array.
 * Returns a NULL string if the token couldn't be found.
 * @param tpath char*
 * @param jsonMsg char*
 * @param jsmnTokenArray jsmntok_t*
 * @return char* the VALUE token found or NULL if not found
 */
char * getTokenValue(char * tpath, char * jsonMsg, jsmntok_t * jsmnTokenArray)
{
	jsmntok_t * jsmnTokenFound;
	char * tokenValue = (char *)NULL;
	char tokenfmt[50];

	jsmnTokenFound = findJsmnToken(tpath, jsonMsg, jsmnTokenArray);
	if (jsmnTokenFound)
	{
		tokenValue = malloc((size_t)getJsmnTokenLen(jsmnTokenFound)+1);
		sprintf(tokenfmt, "%s%lu%s", "%.", (size_t)getJsmnTokenLen(jsmnTokenFound), "s");
		sprintf(tokenValue, tokenfmt, &jsonMsg[jsmnTokenFound->start]);
	}
	return tokenValue;
}
