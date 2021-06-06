#pragma once

// Log defines

extern bool quiet;

#if !defined(NDEBUG)
#define DEBUG_MSG(loc,msg) std::cerr << "[" << loc << "] [DBG] " << msg << "\n"
#else
#define DEBUG_MSG(loc,msg) 
#endif
#define LOG_MSG(loc,msg) if(!quiet) std::cerr << "[" << loc << "] [MSG] " << msg << "\n"
#define LOG_WRN(loc,msg) if(!quiet) std::cerr << "[" << loc << "] [WRN] " << msg << "\n"
#define LOG_ERR(loc,msg) std::cerr << "[" << loc << "] [ERR] " << msg << "\n"

#define LOG_MSG_LINE(loc,msg,line) if(!quiet) std::cerr << "[" << loc << "] [MSG] " << msg << "; Line: \n---> " << line << "\n"
#define LOG_WRN_LINE(loc,msg,line) if(!quiet) std::cerr << "[" << loc << "] [WRN] " << msg << "; Line: \n---> " << line << "\n"
#define LOG_ERR_LINE(loc,msg,line) std::cerr << "[" << loc << "] [ERR] " << msg << "; Line: \n---> " << line << "\n"