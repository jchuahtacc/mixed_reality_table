#ifndef __SERVERCONFIG_HPP__
#define __SERVERCONFIG_HPP__

#include <iostream>
#include <opencv2/core/core.hpp>
#include <string>

namespace mrtable {
    namespace config {
        class ServerConfig {
            public: 
                ServerConfig(string filename="");
                ~ServerConfig();

        };
    }
}

#endif
