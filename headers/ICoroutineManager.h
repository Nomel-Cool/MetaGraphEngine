#pragma once
#ifndef I_COROUTINE_MANAGER_H
#define I_COROUTINE_MANAGER_H

class ICoroutineManager
{
public:
    virtual ~ICoroutineManager() = default;
    virtual bool UpdateAll() = 0;
    virtual bool CheckAllDone() const = 0;
};

#endif // !I_COROUTINE_MANAGER_H
