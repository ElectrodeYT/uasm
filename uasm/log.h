#pragma once

// Log defines

#define LOG_MSG(loc,msg) std::cout << "[" << loc << "] [MSG] " << msg << "\n"
#define LOG_WRN(loc,msg) std::cout << "[" << loc << "] [WRN] " << msg << "\n"
#define QUIT_ERR(loc,msg) std::cout << "[" << loc << "] [ERR] " << msg << "\n"; exit(-1)

#define LOG_MSG_LINE(loc,msg,line) std::cout << "[" << loc << "] [MSG] " << msg << "; Line: \n---> " << line << "\n"
#define LOG_WRN_LINE(loc,msg,line) std::cout << "[" << loc << "] [WRN] " << msg << "; Line: \n---> " << line << "\n"
#define QUIT_ERR_LINE(loc,msg,line) std::cout << "[" << loc << "] [ERR] " << msg << "; Line: \n---> " << line << "\n"; exit(-1)