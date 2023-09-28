//
// Created by consti10 on 08.09.23.
//

#ifndef OPENHD_OPENHDGLOBAL_H
#define OPENHD_OPENHDGLOBAL_H

#include "openhd_action_handler.hpp"

namespace openhd{
class Global {
public:
    static Global& instance();
    openhd::ActionHandler& get_action_handler();
private:
};
}


#endif //OPENHD_OPENHDGLOBAL_H
