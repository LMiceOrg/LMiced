#include "user.h"
#include "sha1.h"

#include <boost/static_assert.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>

#include <fstream>
// include headers that implement a archive in simple text format
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/thread.hpp>

//#include <sys/time.h>
#include <time.h>
#include <string.h>

#include "eal/lmice_eal_common.h"
#include "eal/lmice_eal_time.h"

namespace lmice {

const unsigned char salt[] = "ety2dfhgfj6wg.,m";

BOOST_STATIC_ASSERT( sizeof(time_t) == 8 );

bool lmsession::operator<(const lmsession& s)const
{
    return id < s.id;
}

using namespace boost::multi_index;
typedef multi_index_container<
lmsession,
indexed_by<
ordered_unique<identity<lmsession> >
>
> session_set;

typedef multi_index_container<
lmtoken,
indexed_by<
ordered_unique<identity<lmtoken> >,
ordered_non_unique<member<lmtoken, int32_t, &lmtoken::user_id> >,
ordered_unique<member<lmtoken, int64_t, &lmtoken::conn> >,
ordered_unique<member<lmtoken, std::string, &lmtoken::token> >,
ordered_non_unique<member<lmtoken, std::string, &lmtoken::remote_ip> >

>
> token_set_type;

typedef nth_index<token_set_type, 0>::type token_by_id;
typedef nth_index<token_set_type, 1>::type token_by_user_id;
typedef nth_index<token_set_type, 2>::type token_by_conn;
typedef nth_index<token_set_type, 3>::type token_by_token;
typedef nth_index<token_set_type, 4>::type token_by_remote_ip;


static token_set_type       token_set;
static token_by_id&         token_id_set        = token_set;
//static token_by_user_id&    token_user_id_set   = get<1>(token_set);
static token_by_conn&       token_conn_set      = get<2>(token_set);
//static token_by_token&      token_token_set     = get<3>(token_set);
//static token_by_remote_ip&  token_remote_ip_set = get<4>(token_set);
static boost::shared_mutex token_mutex;

typedef multi_index_container<
lmuser,
indexed_by<
ordered_unique<member<lmuser, int32_t, &lmuser::id> >,
ordered_unique<member<lmuser, std::string, &lmuser::email> >,
ordered_unique<member<lmuser, std::string, &lmuser::name> >
>
> user_set_type;

typedef nth_index<user_set_type, 0>::type user_by_id;
typedef nth_index<user_set_type, 1>::type user_by_email;
typedef nth_index<user_set_type, 2>::type user_by_name;

static user_set_type        user_set;
static user_by_id&          user_id_set         = user_set;
static user_by_email&       user_email_set      = get<1>(user_set);
static user_by_name&        user_name_set       = get<2>(user_set);




bool lmtoken::operator <(const lmtoken &t) const
{
    return id < t.id;
}


void lmtoken::make_challenge()
{
    int64_t tv = 0;
    char buff[64] = {0};
    SHA1Context ctx;

    get_system_time(&tv);

    SHA1Reset(&ctx);
    SHA1Input(&ctx, salt, sizeof(salt));
    SHA1Input(&ctx, (unsigned char*)local_ip.c_str(), local_ip.size());
    SHA1Input(&ctx, (unsigned char*)remote_ip.c_str(), remote_ip.size());
    SHA1Input(&ctx, (unsigned char*)&tv, sizeof(tv));
    SHA1Input(&ctx, salt, sizeof(salt));
    if(!SHA1Result(&ctx))
    {
        throw "sha content corrupted";
    }
#ifdef _MSC_VER
    _snprintf
#else
    snprintf
#endif
    (buff, sizeof(buff), "%08x%08x%08x%08x%08x",
             ctx.Message_Digest[0],
             ctx.Message_Digest[1],
             ctx.Message_Digest[2],
             ctx.Message_Digest[3],
             ctx.Message_Digest[4]
             );
    challenge = buff;
}

void lmtoken::make_token()
{
    int64_t tv = 0;
    char buff[64] = {0};
    SHA1Context ctx;

    get_system_time(&tv);

    SHA1Reset(&ctx);
    SHA1Input(&ctx, salt, sizeof(salt));
    SHA1Input(&ctx, (unsigned char*)&user_id, sizeof(user_id));
    SHA1Input(&ctx, (unsigned char*)challenge.c_str(), challenge.size());
    SHA1Input(&ctx, (unsigned char*)&conn, sizeof(conn));
    SHA1Input(&ctx, (unsigned char*)&tv, sizeof(tv));
    SHA1Input(&ctx, salt, sizeof(salt));
    if(!SHA1Result(&ctx))
    {
        throw "sha content corrupted";
    }
#ifdef _MSC_VER
    _snprintf
#else
    snprintf
#endif
    (buff, sizeof(buff), "%08x%08x%08x%08x%08x",
             ctx.Message_Digest[0],
             ctx.Message_Digest[1],
             ctx.Message_Digest[2],
             ctx.Message_Digest[3],
             ctx.Message_Digest[4]
             );
    token = buff;
}
struct token_change_challenge
{
    token_change_challenge(const std::string & lip, const std::string& rip)
    {
        token.local_ip = lip;
        token.remote_ip = rip;
        token.make_challenge();
    }

    void operator()(lmtoken& e)
    {
        e.challenge = token.challenge;
    }

private:
    lmtoken token;
};

int token_get_challenge(void* conn, const char** challenge)
{
    int64_t ic;
    ic = reinterpret_cast<intptr_t>(conn);
    boost::shared_lock<boost::shared_mutex> lock(token_mutex);
    token_by_conn::iterator ite = token_conn_set.find(ic);
    if(ite != token_conn_set.end())
    {
        *challenge = ite->challenge.c_str();
        return ELMICE_OK;
    }
    return ETOKEN_INVALID_CHALLENGE;
}

int token_compare_challenge(void* conn, const char* challenge)
{
    int64_t ic;
    ic = reinterpret_cast<intptr_t>(conn);
    boost::shared_lock<boost::shared_mutex> lock(token_mutex);

    token_by_conn::iterator ite = token_conn_set.find(ic);
    if(ite != token_conn_set.end())
    {
        if(ite->challenge.compare(challenge) == 0)
            return ELMICE_OK;
    }

    return ETOKEN_INVALID_CHALLENGE;
}

int token_insert(void* conn, const char *local_ip, const char *remote_ip)
{
    static int32_t id = 0;

    boost::unique_lock<boost::shared_mutex> lock(token_mutex);
    {
        lmtoken token;
        token.conn = reinterpret_cast<intptr_t>(conn);
        token.local_ip = local_ip;
        token.remote_ip = remote_ip;
        token.make_challenge();
        token.id = id++;
        token.user_id = 0;
        token.lasttime = 0;



        token_by_conn::iterator ite = token_conn_set.find(token.conn);
        if(ite != token_conn_set.end())
        {
            std::cout<<"conn already exist\n";
            return ETOKEN_CONN_EXIST;
        }
        token_conn_set.insert(token);
    }
    return ELMICE_OK;
}

int token_erase(void* conn)
{
    boost::unique_lock<boost::shared_mutex> lock(token_mutex);
    {
        int64_t ic;
        ic = reinterpret_cast<intptr_t>(conn);
        token_conn_set.erase(ic);
    }
    return 0;
}

int token_reset(void* conn)
{
    int64_t ic;
    ic = reinterpret_cast<intptr_t>(conn);

    boost::unique_lock<boost::shared_mutex> lock(token_mutex);
    token_by_conn::iterator ite = token_conn_set.find(ic);
    if(ite == token_conn_set.end())
        return ETOKEN_CONN_EXIST;
    lmtoken token = *ite;
    token.token = "";
    token.user_id = 0;
    token.make_challenge();
    token_conn_set.replace(ite, token);
    return ELMICE_OK;

}

struct token_change_user
{
    token_change_user(int32_t uid)
        :user_id(uid)
    {
    }

    void operator()(lmtoken& e)
    {
        e.user_id = user_id;
    }

private:
    int32_t user_id;
};
int token_update_user(void* conn, int32_t uid)
{
    int64_t ic;
    ic = reinterpret_cast<intptr_t>(conn);

    boost::shared_lock<boost::shared_mutex> lock(token_mutex);
    token_by_conn::iterator ite = token_conn_set.find(ic);
    token_conn_set.modify(ite, token_change_user(uid));
    return 0;
}

struct token_change_lasttime
{
    token_change_lasttime()
    {
        time(&lasttime);
    }

    void operator()(lmtoken& e)
    {
        e.lasttime = lasttime;
    }

private:
    time_t lasttime;
};

int token_update_time(void* conn)
{
    int64_t ic;
    ic = reinterpret_cast<intptr_t>(conn);

    boost::shared_lock<boost::shared_mutex> lock(token_mutex);
    token_by_conn::iterator ite = token_conn_set.find(ic);
    token_conn_set.modify(ite, token_change_lasttime());
    return ELMICE_OK;
}

int token_size(int* sz)
{
    boost::shared_lock<boost::shared_mutex> lock(token_mutex);
    *sz = token_conn_set.size();
    return 0;
}

/// user set

int user_save_set()
{
    std::ofstream ofs("user_set");
    if(ofs)
    {
        boost::archive::text_oarchive oa(ofs);
        oa<<boost::serialization::make_nvp("mru", user_set);
    }
    return 0;
}

int user_load_set()
{
    std::ifstream ifs("user_set");
    if(ifs)
    {
        boost::archive::text_iarchive ia(ifs);
        ia>>boost::serialization::make_nvp("mru", user_set);
    }
    return 0;
}

int user_add_new(const char* name, const char* display_name, const char* email, const char* password, int* id)
{
    {
        user_by_name::iterator ite = user_name_set.find(name);
        if(ite != user_name_set.end())
            return EUSER_NAME_EXIST;
    }


    {
        *id = user_id_set.size() + 1;
        user_by_id::const_iterator ite = user_id_set.find(*id);
        if(ite != user_id_set.end())
            return EUSER_ID_EXIST;
    }
    {
        user_by_email::const_iterator ite = user_email_set.find(email);
        if(ite != user_email_set.end())
            return EUSER_EMAIL_EXIST;
    }
    user_id_set.insert( lmuser(*id, name, display_name, email, password) );
    return ELMICE_OK;
}

int user_find_name(const char* name, int* id, const char** display_name, const char** email, const char** password)
{
    user_by_name::iterator ite = user_name_set.find(name);
    if(ite == user_name_set.end())
        return -1;
    *id = (ite->id);
    *display_name = (ite->display_name.c_str());
    *email = ite->email.c_str();
    *password = ite->password.c_str();

    return 0;
}

int user_compare_password(int id, const char* challenge, const char* password)
{
    SHA1Context ctx;
    char pass[64] = {0};
    user_by_id::const_iterator ite = user_id_set.find(id);
    if(ite == user_id_set.end())
        return -1;

    SHA1Reset(&ctx);
    SHA1Input(&ctx, (const unsigned char*)challenge, strlen(challenge));
    SHA1Input(&ctx, (const unsigned char*)ite->password.c_str(), ite->password.size());
    SHA1Result(&ctx);
#ifdef _MSC_VER
    _snprintf
#else
    snprintf
#endif
    (pass, 64, "%08x%08x%08x%08x%08x",
             ctx.Message_Digest[0],
             ctx.Message_Digest[1],
             ctx.Message_Digest[2],
             ctx.Message_Digest[3],
             ctx.Message_Digest[4]
             );
    //    std::cout<<"c=\""<<challenge<<"\"\np=\""<<password<<"\"\n"<<
    //                "pass:\t"<<pass<<"\n";
    return strncmp(password, pass, 40);

}

lmuser::lmuser()
{
}

lmuser::lmuser(int32_t i, const std::string & nm, const std::string & dn, const std::string & em, const std::string &pwd)
    :id(i)
    ,name(nm)
    ,display_name(dn)
    ,email(em)
    ,password(pwd)
{
}

} //namespace lmice

namespace boost { namespace serialization {
template<class Archive>
void save(Archive & ar, const lmice::lmuser & t, unsigned int version)
{
    ar << t.id;
    ar << t.name;
    ar << t.password;
    ar << t.email;
    ar << t.display_name;
}
template<class Archive>
void load(Archive & ar, lmice::lmuser & t, unsigned int version)
{
    ar >> t.id;
    ar >> t.name;
    ar >> t.password;
    ar >> t.email;
    ar >> t.display_name;
}


}}

BOOST_SERIALIZATION_SPLIT_FREE(lmice::lmuser)

