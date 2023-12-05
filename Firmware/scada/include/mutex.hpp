#pragma once

namespace scada
{
    template<typename M>
    struct Locker
    {
        Locker(M& mutex) : m_mutex(&mutex){ m_mutex->Lock(); }
        Locker(Locker&& locker) : m_mutex(locker.m_mutex){ locker.m_mutex = nullptr; }
        ~Locker(){ m_mutex->Unlock(); }

        Locker& operator = (Locker&& locker) { 
            m_mutex = locker.m_mutex;
            locker.m_mutex = nullptr;
            return*this;
        }

    private:
        M* m_mutex;
    };

    struct Mutex
    {
    private:
        portMUX_TYPE m_handle;
        Mutex(const Mutex&) = delete;
        Mutex& operator = (const Mutex&) = delete;
        
    public:
        inline Mutex() : m_handle(portMUX_INITIALIZER_UNLOCKED){}
        inline void Lock(){ portENTER_CRITICAL(&m_handle); }
        inline void Unlock(){ portEXIT_CRITICAL(&m_handle); }
        template<typename F>
        inline auto sync(F func) -> decltype(func()) {
            Locker<Mutex> lock = *this;
            return func();
        }
    };
    
}