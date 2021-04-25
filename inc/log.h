#pragma once

// Log defines

#ifdef _DEBUG
#define DEBUG_MSG(loc,msg) std::cout << "[" << loc << "] [DBG] " << msg << "\n"
#else
#define DEBUG_MSG(loc,msg) 
#endif
#define LOG_MSG(loc,msg) std::cout << "[" << loc << "] [MSG] " << msg << "\n"
#define LOG_WRN(loc,msg) std::cout << "[" << loc << "] [WRN] " << msg << "\n"
#define LOG_ERR(loc,msg) std::cout << "[" << loc << "] [ERR] " << msg << "\n"

#define LOG_MSG_LINE(loc,msg,line) std::cout << "[" << loc << "] [MSG] " << msg << "; Line: \n---> " << line << "\n"
#define LOG_WRN_LINE(loc,msg,line) std::cout << "[" << loc << "] [WRN] " << msg << "; Line: \n---> " << line << "\n"
#define LOG_ERR_LINE(loc,msg,line) std::cout << "[" << loc << "] [ERR] " << msg << "; Line: \n---> " << line << "\n"