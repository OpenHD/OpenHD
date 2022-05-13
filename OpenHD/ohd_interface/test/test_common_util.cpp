//
// Created by consti10 on 13.05.22.
//
// Should be placed under ohd_common, test the execute command for weird locale issue

#include "openhd-util.hpp"

int main(int argc, char *argv[]) {

    auto res=run_command("echo",{"1"});
    std::cout<<"Res is:"<<res<<"\n";
}

