/* 
 * File:   http_server.h
 * Author: THORAXIUM
 *
 * Created on March 3, 2015, 8:15 PM
 */

#ifndef HTTP_SERVER_H
#define	HTTP_SERVER_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "FIFO.h"
#include "ESP8266.h"

#define HTTP_CHANNEL_COUNT 5

    typedef enum {
        HTTP_METHOD_UNKNOWN,
        HTTP_METHOD_GET,
        HTTP_METHOD_PUT,
        HTTP_METHOD_POST,
        HTTP_METHOD_DELETE,
        HTTP_METHOD_HEAD,
        HTTP_METHOD_TRACE,
        HTTP_METHOD_CONNECT,
        HTTP_METHOD_OPTIONS
    } HTTP_METHOD;

    typedef enum {
        HTTP_PARSE_STATE_FREE,
        HTTP_PARSE_STATE_START,
        HTTP_PARSE_STATE_HEADERS,
        HTTP_PARSE_STATE_CONTENT,
        HTTP_PARSE_STATE_COMPLETE,
        HTTP_PARSE_STATE_PROCESSING
    } HTTP_PARSE_STATE;

    typedef struct {
        int TCP_ChannelID;
        HTTP_PARSE_STATE ParseState;
        HTTP_METHOD Method;
        unsigned char rawbuffer[1536];
        unsigned char *rawIdx;
        char *Resource;
        char *ETag;
        unsigned long expires;
        int ContentLength;
        int ContentRecieved;
        BYTE *Content;
        char *nonce;
        char *cnonce;
        char *nonceCount;
        char *response;
    } HTTP_REQUEST;

    typedef struct {
        const char *InterfaceName;
        void (*OnGet)(HTTP_REQUEST *req, const char *urlParameter);
        int GetAuth;
        void (*OnPut)(HTTP_REQUEST *req, const char *urlParameter);
        int PutAuth;
        int CfgModeRequired;
    } API_INTERFACE;

    int url_queryParse(const char *queryString, const char *parameter, char **output, int *len);
    int url_queryParse2(const char *queryString, const char *parameter, char *output, int MaxLen);
    void HTTP_ServerLoop();
    void ParseMessage_HTTP(MESSAGE *msg);
    void Send500_InternalServerError(HTTP_REQUEST * req, const char *Msg);
    void Send200_OK_Simple(HTTP_REQUEST * req);
    void Process_GET_File(HTTP_REQUEST * req);
    void Send200_OK_SmallMsg(HTTP_REQUEST * req, const char *msg);
    void Send501_NotImplemented(HTTP_REQUEST * req);
    void Send404_NotFound(HTTP_REQUEST * req);
    void Process_GET_File_ex(HTTP_REQUEST * req, unsigned long start, unsigned long length);
    void Send200_OK_Data(HTTP_REQUEST * req, unsigned char *msg, int length);
    char MakeLowercase(char inp);
#ifdef	__cplusplus
}
#endif

#endif	/* HTTP_SERVER_H */

