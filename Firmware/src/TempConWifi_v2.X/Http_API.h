/* 
 * File:   Http_API.h
 * Author: THORAXIUM
 *
 * Created on February 7, 2015, 10:28 PM
 */

#ifndef HTTP_API_H
#define	HTTP_API_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "integer.h"
#include "http_server.h"
#include <stddef.h>

    void BuildProfileInstanceListing(const char *ProfileName);
    int UpdateCredentials(const char*username, const char *password);
    void ProcessAPI(HTTP_REQUEST *req, API_INTERFACE *API);
    API_INTERFACE *GetAPI(HTTP_REQUEST *req);

#ifdef	__cplusplus
}
#endif

#endif	/* HTTP_API_H */

