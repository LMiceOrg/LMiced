#ifndef LMPROFILE_H
#define LMPROFILE_H

#include <stdint.h>
#include <string>

namespace lmice {

struct lmprofile {
int32_t id;
std::string name;
int32_t user_id;
std::string data_source;
std::string profile;
};

}//namespace lmice

#endif // LMPROFILE_H
