# TanmiEventSystem

> 2023.4.17 update

TanmiEventSystem是一个基于观察者模式实现的异步事件系统，可以让你方便地在程序中广播事件并将其传递给所有已注册的监听器。
该系统通过mutex实现线程安全，支持从多个线程同时访问。

本事件系统通过绑定和注销事件至监听器实现事件分发，并在触发时异步分发对应事件至指定监听器。
在使用时钟相关的事件时，结合[TanmiClock](https://github.com/tanmika/TanmiClock)时钟系统，实现时钟更新时自动唤起事件功能。

## 依赖

src文件夹下包含：
- TanmiEventSystem.hpp
事件系统
- TanmiMessageHandler.hpp
消息处理器
- TanmiMessageQuene.hpp
消息队列
- TanmiEvent.hpp
事件类
- TanmiListener.hpp
事件监听器接口

- TanmiClock.hpp
时钟系统

demo文件夹下包含：
- TanmiEventSystem.sln
演示项目

将TanmiEventSystem.hpp与TanmiListener.hpp包含至项目中即可使用，
若需要时钟功能，还需包含TanmiClock.hpp，调用时需使用命名空间TanmiEngine

注意，在使用该项目时需要支持 C++20 特性的编译器。

## 结构
    - EventSystem：事件系统 用于管理事件的预处理与是否分发，包含两个事件处理器
		- MessageHandler：异步事件处理器 用于管理事件的分发，包含二至三个消息队列
			- MessageQuene：消息队列 用于存储事件的某一元素
    - Event：事件 用于标志一类事件的触发，包含一个预处理函数
	- Listener：监听器 用于监听事件的触发，包含两个触发函数

	- Clock ：时钟 用于管理时钟的更新与事件的触发

## 用法
- **事件**
```c++
// 默认类型事件，其预处理函数永远返回true
auto event = std::make_shared<Event>();
// 自定义类型事件
class MyEvent : public Event
{
public:
	MyEvent() {}
	~MyEvent() {}
	// 事件预处理函数，返回true则触发事件，返回false则不触发事件
	virtual bool PreProcess() 
	{
		// 事件预处理
		if(/*something*/)
			return true;
		return false;
	}
};
// 注册事件
auto myEvent = std::make_shared<MyEvent>();
```
- **事件监听器**
```c++
// 默认监听器为抽象类，必须被构建
class MyListener : public Listener
{
public:
	MyListener() {}
	~MyListener() {}
	// 事件触发时回调
	virtual void WakeEvent(const EventID event);
	// 事件触发时回调，带有事件触发时距上次调用经过的时间
	virtual void WakeEventUpdate(const EventID event, double elapesd_ms);
};
// 注册监听器
auto listener = std::make_shared<MyListener>();
```
- **消息处理器**
```c++
// !消息处理器一般不用显式定义
// 默认消息处理器，处理直接触发的事件
auto messageHandler = std::make_shared<MessageHandler>();
// 默认消息处理器（带有时间参数），处理带有时间参数的事件
auto messageHandlerUpdate = std::make_shared<MessageHandlerUpdate>();

// 默认消息处理器功能不够时，可通过自定义消息处理实现多并发，优先级队列，超时处理等功能
```
- **事件系统**
```c++
// 获取EventSystem实例引用
EventSystem& eventSystem = EventSystem::Instance();
```
- 事件系统需绑定事件处理器后才可使用
```c++
// 使用默认消息处理器
auto msgHandler = eventSystem.RegisterMessageHandler<MessageHandler>();
auto msgHandlerUpdate = eventSystem.RegisterMessageHandlerUpdate<MessageHandlerUpdate>();
// 或 (推荐)
eventSystem.UseDefaultMessageHandler();
```
- 事件注册后才可绑定
```c++
// 注册事件testEvent
auto testEvent = std::make_shared<Event>();
eventSystem.RegisterEvent(testEvent);
// 或（推荐）
auto testEvent = eventSystem.NewAndRegisterEvent<Event>();

// 注册10个事件testEvents
auto testEvents = eventSystem.NewAndRegisterEvents<Event>(10);
// 或
auto testEvents = eventSystem.NewAndRegisterEvents<Event, std::vector>(10);
```
- 事件绑定
```c++
// 绑定事件testEvent至监听对象listener
eventSystem.AddEventHandler(*testEvent, listener);

// 绑定事件vector testEvents至监听对象listener
eventSystem.AddEventHandler(testEvents, listener);

// 绑定事件testEvent至监听对象vector listeners
eventSystem.AddEventHandler(*testEvent, listeners);
```
- 事件触发
```c++
// 触发事件
eventSystem.TriggerEvent(testEvent);

// 触发事件，带有事件触发时距上次调用经过的时间
eventSystem.TriggerEventUpdate(testEvent, 1000);

// 移除事件下特定监听器
eventSystem.RemoveEventHandler(testEvent, listener);

// 移除事件下所有监听器
eventSystem.RemoveAllEventForEventName(testEvent);

// 移除所有事件下特定监听器
eventSystem.RemoveAllEventForListener(listener);

// 判断事件是否存在
bool isExist = eventSystem.IsEventExist(testEvent);

// 判断事件是否存在，无异常检测
bool isExistNoExpection = eventSystem.IsEventExistNoExcept(testEvent);
```
- 时钟相关事件

*需包含[TanmiClock](https://github.com/tanmika/TanmiClock)实现*

```c++
// 新建时钟testClk
Clock& clock = Clock::Instance();
auto testClk = clock.NewClock();

// 添加事件testEvent至时钟testClk
clock.AddEvent(testClk, *testEvent);

// 添加事件testEvents vector至时钟testClk
clock.AddEvent(testClk, testEvents);

// 移除事件testEvent由时钟testClk
clock.RemoveEvent(testClk, *testEvent);

// 获取时钟testClk下所有事件
std::vector<EventID> eventList = clock.GetEventList(testClk);

// 移除时钟testClk下所有事件
clock.ClearEventList(testClk);
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
- 事件
```c++
class Event
{
public:
	// 构造函数，初始化事件ID为0
	Event() :ID(0)
	{}
	// 默认析构函数
	~Event() = default;
	// 预处理函数，用于实现预处理逻辑
	virtual bool preProcess()
	{
		return true;
	}
	EventID ID;	// 事件ID
};
```
- 消息处理器
```c++
class MessageHandler
{
public:
	MessageHandler() = default;		//	默认构造函数
	~MessageHandler() = default;	//	默认析构函数
	// 发送事件
	virtual void Post(EventID id, std::shared_ptr<Listener> cilent)
	{
		listener.Push(cilent);
		listener_id.Push(id);
	}
	// 处理消息队列
	virtual void Run()
	{
		while (!exit)
		{
			HandleMessage(listener_id.Pop(), std::move(listener.Pop()));
		}
	}
	// 处理消息
	virtual void HandleMessage(const EventID id, std::shared_ptr<Listener> message)
	{
		message->WakeEvent(id);
	}
	// 关闭消息处理器
	void Exit()
	{
		exit = true;
	}
protected:
	MessageQueue<std::shared_ptr<Listener>> listener;	// 监听器队列
	MessageQueue<EventID> listener_id;					// 事件ID队列
	bool exit = false;									// 是否退出
};
// 带有时间参数
class MessageHandler
{
public:
	MessageHandler() = default;		//	默认构造函数
	~MessageHandler() = default;	//	默认析构函数
	// 发送事件
	virtual void Post(EventID id, std::shared_ptr<Listener> cilent, double ms)
	{
		listener.Push(cilent);
		listener_id.Push(id);
		listener_t.Push(ms);
	}
	// 处理消息队列
	virtual void Run()
	{
		while (!exit)
		{
			HandleMessage(listener_id.Pop(), std::move(listener.Pop()), listener_t.Pop());
		}
	}
	// 处理消息
	virtual void HandleMessage(const EventID id, std::shared_ptr<Listener> message, double time)
	{
		message->WakeEvent(id);
	}
	// 关闭消息处理器
	void Exit()
	{
		exit = true;
	}
protected:
	MessageQueue<std::shared_ptr<Listener>> listener;	// 监听器队列
	MessageQueue<EventID> listener_id;					// 事件ID队列
	MessageQueue<double> listener_t;					// 事件发生所经过的时间（以毫秒为单位）
	bool exit = false;									// 是否退出
};
```