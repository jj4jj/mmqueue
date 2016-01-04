#pragma  once
#include "stdinc.h"
#include "dcobjects.hpp"

NS_BEGIN(dcsutil)
    //-----------------noncopy----------------------------
	class noncopyable
	{
	protected:
		noncopyable() {}
		~noncopyable() {}
	private: // emphasize the following members are private  
		noncopyable(const noncopyable&);
		const noncopyable& operator=(const noncopyable&);
	};
    //-----------------lock-------------------------------
    template<bool threadsafe>
    struct lock_mixin;

    template<>
    struct lock_mixin<false>{
        void lock(){}
        void unlock(){}
    };

    template<>
    struct lock_mixin<true>{
        void lock(){ lock_.lock(); }
        void unlock(){ lock_.unlock(); }
    private:
        std::mutex  lock_;
    };
    //-------------------------------------------------------------------------
    //----------misc------------------------------------------------------------
	//time 
	uint64_t			time_unixtime_us();
	inline	time_t		time_unixtime_s(){ return time_unixtime_us() / 1000000L; }
	inline	uint64_t	time_unixtime_ms(){ return time_unixtime_us() / 1000L;}
	const char*			strftime(std::string & str, time_t unixtime = 0, const char * format = "%FT%X%z");
	const char*			strptime(time_t & unixtime, const std::string & str, const char * format = "%FT%X%z");
	time_t				stdstrtime(const char * strtime = "1970-01-01T08:08:08+0800");

	//file releataed
    //if sz = 0 , test file exist
    int					readfile(const std::string & file, char * buffer = 0, size_t sz = 0);
    int					writefile(const std::string & file, const char * buffer, size_t sz = 0);
    size_t              filesize(const std::string & file);

    //file://<path>
    //tcp://<ip:port>
    //udp://<ip:port>
    int                 openfd(const std::string & uri, int timeout_ms = 30000);
    //mode: size, end, msg:sz32/16/8, token:\r\n\r\n , return > 0 read size, = 0 end, < 0 error
    int                 readfd(int fd, char * buffer, size_t sz, const char * mode, int timeout_ms = 10000);
    //write size , return > 0 wirte size, <= 0 error
    int                 writefd(int fd, const char * buffer, size_t sz, int timeout_ms = 2000);
    int                 closefd(int fd);
    int                 nonblockfd(int fd, bool nonblock = true);
    //return 0: readable , other error occurs
    int                 waitfd_readable(int fd, int timeout_ms);
    int                 waitfd_writable(int fd, int timeout_ms);
    int                 ipfromhostname(OUT uint32_t * ip, INOUT int & ipnum, const std::string & hostname);
    int                 socknetaddr(struct sockaddr_in & addr, const std::string & saddr);
    ///////////process////////////////////////////////////////////////////////////////////////
	int					daemonlize(int closestd = 1, int chrootdir = 0);
	//-1:open file error , getpid():lock ok , 0:lock error but not known peer, >0: the locker pid.
    int					lockpidfile(const std::string & pidfile, int kill_other_sig = 0, bool nb = true);

	///////////str////////////////////////////////////////////////////////////////////////////////
	int					strsplit(const std::string & str, const string & sep, std::vector<std::string> & vs, bool ignore_empty = true, int maxsplit = 0, int beg = 0, int end = 0);
	size_t				strprintf(std::string & str, const char * format, ...);
	size_t				strnprintf(std::string & str, size_t max_sz, const char * format, ...);
	size_t				vstrprintf(std::string & str, const char* format, va_list va);
	void				strrepeat(std::string & str, const char * rep, int repcount);
    bool                strisint(const std::string & str, int base = 10);
    const char *		strrandom(std::string & randoms, int length = 8, char charbeg = 0x21, char charend = 0x7E);
    const char *		strcharsetrandom(std::string & randoms, int length = 8, const char * charset = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIGKLMNOPQRSTUVWXYZ_!@#$+-");

    template <typename StrItable>
    const char         *strjoin(std::string & val, const std::string & sep, StrItable it){
        size_t i = 0;
        val.clear();
        for (auto & v : it){
            if (i != 0){v.append(sep);}
            v.append(v);
            ++i;
        }
        return val.c_str();
    }
    const char          *strspack(std::string & str, const std::string & sep, const std::string & ks, ...);
    int                 strsunpack(const std::string & str, const std::string & sep, const std::string & k, ...);
    //todo variadic  



    ///////////uuid////////////////////////////////////////////////////////////////////////////////////////////////


    ///============================================================
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////

NS_END()
