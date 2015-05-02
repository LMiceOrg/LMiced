#include "sha1.h"
#include "user.h"

#include <jansson.h>
#include <string.h>

#include <stdexcept>
#include <string>
#include <map>
#include <iostream>
#include <sstream>

//#include <sys/time.h>
#include <time.h>

#define json_object_get_string(root, obj, name, value) do {   \
    obj = json_object_get(root, name);  \
    if(!obj) {   \
    throw std::runtime_error("Request did not contain name param");   \
    }   \
    value = json_string_value(obj); \
    }while(0)

#define json_object_set_string(root, name, src) do {  \
    json_t * str = json_string_nocheck( src);    \
    json_object_set_new_nocheck(root, name, str);   \
    }while(0)

#define json_object_set_bool(root, name, src) do {  \
    json_t * str; \
    if(src) \
        str = json_true();  \
    else    \
        str = json_false(); \
    json_object_set_new_nocheck(root, name, str);   \
    }while(0)

//std::map<int64_t, lmtoken*> g_challenge;

char* parse_response(void* conn, const char* request, int length)
{
    json_t* root = NULL;
    json_t* obj = NULL;
    json_error_t err;
    char* response = NULL;
    std::string resaction = "errorResponse";
    try
    {

        memset(&err, 0, sizeof(json_error_t));
        root = json_loadb(request, length, JSON_DECODE_ANY, &err);
        if(!root)
        {
            throw std::runtime_error("Request is not a valid JSON object");
        }

        const char* action;
        json_object_get_string(root, obj, "action", action);
        if(!action)
        {
            throw std::runtime_error("Request type is unknown");
        }
        else if(strcmp(action, "loginType1") == 0)
        {
            int ret;
            const char* challenge;
            resaction = "loginType1Response";
            ret = lmice::token_get_challenge(conn, &challenge);
            if(ret != 0)
                throw std::runtime_error("Client connection is invalid");

            json_t* rep = json_object();
            json_object_set_string(rep, "action", resaction.c_str());
            json_object_set_bool(rep, "status", true);
            json_object_set_string(rep, "challenge", challenge);
            json_object_set_string(rep, "error", "OK");
            response = json_dumps(rep, JSON_INDENT(4));
            json_decref(rep);
        }
        else if(strcmp(action, "loginType2") == 0)
        {
            int id;
            int ret;
            const char* name;
            const char* pass;
            const char* challenge;
            const char* email;
            const char* password;
            const char* display_name;

            resaction = "loginType2Response";


            json_object_get_string( root, obj, "name", name);
            json_object_get_string( root, obj, "password", pass);
            json_object_get_string( root, obj, "challenge", challenge);

            ret = lmice::token_compare_challenge(conn, challenge);
            if(ret != 0)
                throw std::runtime_error("Client status is incorrect");

            ret = lmice::user_find_name(name, &id, &display_name, &email, &password);
            if(ret != 0)
                throw std::runtime_error("User name is invalid");

            ret = lmice::user_compare_password(id, challenge, pass);
            if( ret != 0)
                throw std::runtime_error("Password is incorrect");

            lmice::token_update_user(conn, id);

            json_t* rep = json_object();
            json_object_set_string(rep, "action", resaction.c_str());
            json_object_set_bool(rep, "status", true);
            json_object_set_string(rep, "token", "123");
            json_object_set_string(rep, "displayName", display_name);
            json_object_set_string(rep, "email", email);
            json_object_set_string(rep, "error", "OK");

            response = json_dumps(rep, JSON_INDENT(4));
            json_decref(rep);
        }
        else if(strcmp(action, "logout") == 0)
        {
            //  Logout
            const char* token;
            json_object_get_string(root, obj, "token", token);

            lmice::token_reset(conn);
            if(!token)
            {
                throw std::runtime_error("Client status is incorrect");
            }
            json_t* rep = json_object();
            json_object_set_string(rep, "action", "logoutResponse");
            json_object_set_bool(rep, "status", true);
            json_object_set_string(rep, "error", "OK");
            response = json_dumps(rep, JSON_INDENT(4));
            json_decref(rep);

        }
        else if(strcmp(action, "register") == 0)
        {
            //  Register
            const char* name;
            const char* display_name;
            const char* password;
            const char* email;
            int ret;
            int id;

            resaction = "registerResponse";
            json_object_get_string( root, obj, "name", name);
            json_object_get_string( root, obj, "password", password);
            json_object_get_string( root, obj, "display_name", display_name);
            json_object_get_string( root, obj, "email", email);

            if(!name)
                throw std::runtime_error("Name is null");
            if(!password)
                throw std::runtime_error("Password is null");
            if(!display_name)
                display_name = name;
            if(!email)
                throw std::runtime_error("Email is null");

            ret = lmice::user_add_new( name, display_name, email, password, &id);
            switch(ret)
            {
            case lmice::EUSER_ID_EXIST:
                throw std::runtime_error("Create new user failed(ID)");
            case lmice::EUSER_NAME_EXIST:
                throw std::runtime_error("Create new user failed(Name)");
            case lmice::EUSER_EMAIL_EXIST:
                throw std::runtime_error("Create new user failed(Email)");
            }

            json_t* rep = json_object();
            json_object_set_string(rep, "action", "registerResponse");
            json_object_set_bool(rep, "status", true);
            json_object_set_string(rep, "error", "OK");
            response = json_dumps(rep, JSON_INDENT(4));
            json_decref(rep);


        }
    }
    catch(const std::exception& e)
    {
        json_t* rep = json_object();
        json_object_set_string(rep, "action", resaction.c_str());
        json_object_set_bool(rep, "status", false);
        json_object_set_string(rep, "error", e.what());
        response = json_dumps(rep, JSON_INDENT(4));
        json_decref(rep);
    }

    json_decref(root);
    return response;
}
