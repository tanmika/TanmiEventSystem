# TanmiEventSystem

> 2023.4.8 update

TanmiEventSystem是一个基于观察者模式实现的事件系统，可以让你方便地在程序中广播事件并将其传递给所有已注册的监听器。
该系统通过mutex实现线程安全，支持从多个线程同时访问。

本事件系统通过绑定和注销事件至监听器实现事件分发，并在触发时分发对应事件至指定监听器。
在使用时钟相关的事件时，结合[TanmiClock](https://github.com/tanmika/TanmiClock)时钟系统，实现时钟更新时自动唤起事件功能。

## 依赖

src文件夹下包含：
- TanmiEventSystem_64.hpp
事件系统
- TanmiListener_64.hpp
事件监听器接口
- TanmiClock_64.hpp
时钟系统

demo文件夹下包含：
- TanmiEventSystem.sln
演示项目

将TanmiEventSystem_64.hpp与TanmiListener_64.hpp包含至项目中即可使用，
若需要时钟功能，还需包含TanmiClock_64.hpp，调用时需使用命名空间TanmiEngine

注意，在使用该项目时需要支持 C++20 特性的编译器。
## 用法
- 事件监听器
```c++
class MyListener : public Listener
{
public:
	MyListener() {}
	~MyListener() {}
	// 事件触发时回调
	virtual void WakeEvent(const std::string& event);
	// 事件触发时回调，带有事件触发时距上次调用经过的时间
	virtual void WakeEventUpdate(const std::string& event, double elapesd_ms);
};
// 注册监听器
auto listener = std::make_shared<MyListener>();
```
- 事件系统
```c++
// 获取EventSystem实例引用
EventSystem& eventSystem = EventSystem::Instance();

// 添加事件testEvent至事件系统
eventSystem.AddEventHandler("testEvent", listener);

// 触发事件
eventSystem.TriggerEvent("testEvent");

// 触发事件，带有事件触发时距上次调用经过的时间
eventSystem.TriggerEventUpdate("testEvent", 1000);

// 移除事件下特定监听器
eventSystem.RemoveEventHandler("testEvent", listener);

// 移除事件下所有监听器
eventSystem.RemoveAllEventForEventName("testEvent");

// 移除所有事件下特定监听器
eventSystem.RemoveAllEventForListener(listener);

// 判断事件是否存在
bool isExist = eventSystem.IsEventExist("testEvent");

// 判断事件是否存在，无异常检测
bool isExistNoExpection = eventSystem.IsEventExistNoExcept("testEvent");
```
- 时钟相关事件

*需包含[TanmiClock](https://github.com/tanmika/TanmiClock)实现*

```c++
// 新建时钟testClk
Clock& clock = Clock::Instance();
clock.NewClock("testClk");

// 添加事件testEvent至时钟testClk
clock.AddEvent("testClk", "testEvent");

// 移除事件testEvent由时钟testClk
clock.RemoveEvent("testClk", "testEvent");

// 获取时钟testClk下所有事件
std::vector<std::string> eventList = clock.GetEventList("testClk");

// 移除时钟testClk下所有事件
clock.ClearEventList("testClk");
```

## 接口
- 事件监听器
```c++
class Listener
{
public:
	Listener() = default;
	~Listener() = default;
	// 事件触发
	// event:事件名称
	virtual void WakeEvent(const std::string& event) = 0;
	// 事件触发
	// event:事件名称
	// ms:触发时间间隔
	virtual void WakeEventUpdate(const std::string& event, double ms) = 0;
};
```
- 事件系统
```c++
class EventSystem
{
...
public:
	// 获取EventSystem实例引用
	static EventSystem& Instance();

	// 触发事件
	// event: 事件名称
	void TriggerEvent(const std::string& event);

	// 触发事件
	// event: 事件名称
	// ms: 事件触发时的更新间隔
	void TriggerEventUpdate(const std::string& event, double ms);

	// 添加事件
	// event: 事件名称
	// client: 监听对象
	void AddEventHandler(const std::string& event, std::shared_ptr<Listener> client);

	// 移除事件下特定监听
	// event: 事件名称
	// client: 待移除的监听对象
	void RemoveEventHandler(const std::string& event, std::shared_ptr<Listener> client);

	// 移除指定事件名称下的所有监听
	// event: 事件名称
	void RemoveAllEventForEventName(const std::string& event);

	// 移除所有与指定监听对象相关的事件
	// client: 待移除的监听对象
	void RemoveAllEventForListener(const std::shared_ptr<Listener> client);

	// 判断指定事件是否存在
	// event: 事件名称
	bool IsEventExist(const std::string& event)const;
		
	// 判断指定事件是否存在，无异常检测
	// event: 事件名称
	bool IsEventExistNoException(const std::string& event)const;
	...
};
```
- 时钟相关事件
```c++
class Clock
{
    ...
public:
	...
#ifdef EVENT_SYSTEM
	// 添加事件至时钟
	void AddEvent(const std::string&, const std::string& event);
	// 移除事件
	void RemoveEvent(const std::string&, const std::string& event);
	// 获取事件列表
	std::vector<std::string>& GetEventList(const std::string&);
	// 清空事件列表
	void ClearEventList(const std::string&);
#endif // EVENT_SYSTEM
...
};
```
