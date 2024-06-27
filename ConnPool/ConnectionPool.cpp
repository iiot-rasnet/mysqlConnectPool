#include "ConnectionPool.h"
#include <json/json.h>
#include <fstream>
#include <thread>
using namespace Json;
ConnectionPool* ConnectionPool::getConnetPool()
{
    static ConnectionPool pool;
    return &pool;
}

//消费者进程
shared_ptr<MysqlConn> ConnectionPool::getConnection()
{
    unique_lock<mutex>locker(m_mutexQ);
    while (m_connectionQ.empty()) {
        if (cv_status::timeout == m_cond.wait_for(locker, chrono::milliseconds(m_timeout))) {
            if (m_connectionQ.empty()) {
                //return nullptr;
                continue;
            }
        }
    }
    shared_ptr<MysqlConn> connptr(m_connectionQ.front(), [this](MysqlConn* conn){
        lock_guard<mutex>locker(m_mutexQ);
        conn->refreshAliveTime();
        m_connectionQ.push(conn);
        });
    m_connectionQ.pop();
    m_cond.notify_all();
    return connptr;
}

ConnectionPool::~ConnectionPool()
{
    while (!m_connectionQ.empty()) {
        MysqlConn* conn = m_connectionQ.front();
        m_connectionQ.pop();
        delete conn;
    }
}

bool ConnectionPool::parseJsonFile()
{
    ifstream ifs("dbconf.json");
    Reader rd;
    Value root;
    rd.parse(ifs, root);
    if (root.isObject()) {
        m_ip = root["ip"].asString();
        m_port = root["port"].asInt();
        m_user = root["userName"].asString();
        m_passwd = root["password"].asString();
        m_dbName = root["dbName"].asString();
        m_minSize = root["minSize"].asInt();
        m_maxSize = root["maxSize"].asInt();
        m_maxIdleTime = root["maxIdleTime"].asInt();
        m_timeout = root["timeout"].asInt();
        return true;
    }
    return false;
}


//生产者线程
void ConnectionPool::produceConnection()
{
    while (true) {
        unique_lock<mutex> locker(m_mutexQ);
        while (m_connectionQ.size() >= m_minSize) {
            m_cond.wait(locker);
        }
        m_cond.notify_all();
    }
    }
       

void ConnectionPool::recycleConnection()
{
    
    while (true) {
        this_thread::sleep_for(chrono::seconds(1));
        lock_guard<mutex> locker(m_mutexQ);
        while (m_connectionQ.size() > m_minSize) {
            MysqlConn* conn = m_connectionQ.front();
            if (conn->getAliveTime() >= m_maxIdleTime) {
                m_connectionQ.pop();
                delete conn;
            }
            else {
                break;
            }
        }
    }
}

void ConnectionPool::addConnection()
{
    MysqlConn* conn = new MysqlConn;
    conn->connect(m_user, m_passwd, m_dbName, m_ip, m_port);
    conn->refreshAliveTime();
    m_connectionQ.push(conn);
}

//数据库连接池
ConnectionPool::ConnectionPool() {

    //加载配置文件
    if (!parseJsonFile()) {
        cout << "加载配置文件失败";
        return;
    }

    for (int i = 0; i < m_minSize; ++i) {
        addConnection();
    }
    thread producer(&ConnectionPool::produceConnection, this);
    thread recycler(&ConnectionPool::recycleConnection, this);
    producer.detach();;
    recycler.detach();
}
