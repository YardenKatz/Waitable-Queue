#ifndef __WAITABLE_QUEUE__
#define __WAITABLE_QUEUE__

#include <iostream>
#include <queue>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

namespace ilrd
{
using namespace boost::chrono; 

// T must be copyable & assignable
// Q must supply push, pop, front or supply function instead of front
template <class T, class Q = std::queue<T> >
class WaitableQueue: private boost::noncopyable
// REENTRANT
{
public:
    //  WaitableQueue(); = generated
    // ~WaitableQueue(); = generated
    
    void Enqueue(const T& data); 
    void Deque(T& out); 
	
	// returns false if returns on timeout
    bool Deque(T& out, nanoseconds timeout); 
	bool IsEmpty();
    
private:
    Q m_container;
    boost::mutex m_mutex;
    boost::condition_variable m_cond;
};

template <class T, class Q>
void WaitableQueue<T, Q>::Enqueue(const T& data)
{
    boost::unique_lock<boost::mutex> lock(m_mutex);
    m_container.push(data);
    m_cond.notify_all();
}

template <class T, class Q>
bool WaitableQueue<T, Q>::Deque(T& out, nanoseconds timeout)
{
	system_clock::time_point timeoutAbs(system_clock::now() + timeout);

    //mutex lock (auto unlock on exit)
    boost::unique_lock<boost::mutex> lock(m_mutex);
    
    //cond var - wait until timeout exipred or container isn't empty
    while (m_container.empty())
    {
        // if timeout expired return false, else iterate to another wait_for 
        if (boost::cv_status::timeout == m_cond.wait_until(lock, timeoutAbs))
        {
            return false;
        }
    }

    // read front & pop
    out = m_container.front();
    m_container.pop();

    return true;
}

template <class T, class Q>
void WaitableQueue<T, Q>::Deque(T& out)
{
    Deque(out, static_cast<hours>(500));
}

template <class T, class Q>
bool WaitableQueue<T, Q>::IsEmpty()
{
	return m_container.empty();
}

} // namespace ilrd

#endif // __WAITABLE_QUEUE__