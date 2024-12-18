#pragma once
#ifndef DATABASE_H
#define DATABASE_H

#include <mysql.h>
#include <string>
#include <mutex>
#include <stdexcept>
#include <iostream>
#include <optional>
#include <chrono>
#include <thread>
#include <atomic>

class MySQLDatabase
{
public:
    // 获取单例实例
    static MySQLDatabase& GetInstance();

    // 初始化数据库连接
    void Init(const std::string& host, const std::string& user, const std::string& password, const std::string& dbname);

    // 断开连接并停止心跳
    void Disconnect();

    // 执行查询
    std::optional<MYSQL_RES*> Query(const std::string& query_str);

    // 执行增、改、删操作
    bool Execute(const std::string& query_str);

    // 检查是否连接
    bool IsConnected();

private:
    MySQLDatabase() : m_conn(nullptr), m_heartbeatActive(false) {}
    ~MySQLDatabase() { Disconnect(); }
    // 删除拷贝构造函数和赋值运算符，以确保单例模式
    MySQLDatabase(const MySQLDatabase&) = delete;
    MySQLDatabase& operator=(const MySQLDatabase&) = delete;

    // 连接并重试直到成功
    void ConnectWithRetry();

    // 心跳线程逻辑
    void heartbeatCheck();

    MYSQL* m_conn;
    std::string m_host, m_user, m_password, m_dbname;
    std::mutex m_mutex; // 保护数据库连接的线程安全

    std::atomic<bool> m_heartbeatActive; // 控制心跳线程的运行状态
    std::thread m_heartbeatThread;       // 心跳线程
};

#endif // DATABASE_H
