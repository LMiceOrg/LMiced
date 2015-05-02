#ifndef USER_H
#define USER_H

#include <stdint.h>
#include <string>
namespace lmice {

// 任务设定
struct lmtask_profile
{
    int32_t id;
};

// 用户设定
struct lmuser_profile
{
    int32_t id;
};

// 内部接口 用户
struct lmuser
{
    int32_t id;

    std::string name;
    std::string display_name;
    std::string email;
    std::string password;

    lmuser_profile* profile;
    lmuser();
    lmuser(int32_t, const std::string&, const std::string&, const std::string&, const std::string&);
};

struct lmtask
{
    int32_t id;
    lmtask_profile *profile;
};

// 任务会话
struct lmsession
{
    int32_t id;
    bool operator<(const lmsession& s)const;

};

// 外部接口 凭证
struct lmtoken
{
    int32_t id;

    int32_t user_id;
    std::string challenge;  // sha1-string
    std::string token;      // sha1-string
    std::string local_ip;   // ipv6-ipv4
    std::string remote_ip;  // ipv6-ipv4
    int64_t conn;
    time_t  lasttime;

    bool operator <(const lmtoken& t) const;
    void make_challenge();
    void make_token();
};
enum
{
    ELMICE_OK = 0,

    EUSER_NAME_EXIST = 1,
    EUSER_EMAIL_EXIST,
    EUSER_ID_EXIST,

    ETOKEN_INVALID_CHALLENGE,
    ETOKEN_CONN_EXIST
};
int token_get_challenge(void* conn, const char** challenge);
int token_compare_challenge(void* conn, const char* challenge);
int token_insert(void* conn, const char* local_ip, const char* remote_ip);
int token_erase(void* conn);
int token_reset(void* conn);
int token_update_user(void* conn, int32_t uid);
int token_update_time(void* conn);
int token_size(int* sz);


int user_save_set();
int user_load_set();
int user_add_new(const char* name, const char* display_name, const char* email, const char* password, int*id);
int user_find_name(const char* name, int* id, const char** display_name, const char** email, const char** password);
int user_compare_password(int id, const char* challenge, const char* password);

} // namespace lmice

#endif // USER_H
