/*
	programmer: du yan ning
	date: 2005-10-29
	date: 2019-07-05 add support to lambda
*/
#ifndef EVENT_TYPE_HPP
#define EVENT_TYPE_HPP

#include <list>

namespace Shutters {

class AbstractEventHandlerBase {
public:
	AbstractEventHandlerBase()
		: ownerIsEvent_(true), eventIsAlive_(true)
	{
	}

	bool ownerIsEvent()
	{
		return ownerIsEvent_;
	}
	
	void ownerIsEvent(bool is)
	{
		ownerIsEvent_ = is;
	}

	bool eventIsAlive()
	{
		return eventIsAlive_;
	}

	void eventIsAlive(bool is)
	{
		eventIsAlive_ = is;
	}
private:
	bool ownerIsEvent_;
	bool eventIsAlive_;
};

template <class Target, class MF, class AbstractEventHandler>
class EventHandlerBase : public AbstractEventHandler {
public:
	EventHandlerBase(Target* target, MF mf)
		: target_(target), mf_(mf)
	{
	}

protected:
	Target* target_;
	MF mf_;
};

template <class Functor, class AbstractEventHandler>
class EventFunctorHandlerBase : public AbstractEventHandler {
	void operator=(const EventFunctorHandlerBase&);	// to avoid warning in /W4
public:
	EventFunctorHandlerBase(Functor& f)
		: f_(f)
	{
	}

protected:
	Functor& f_;
};

template <
		class AbstractEventHandler,
		template <class, class> class ConcreteEventHandler,
		template <class> class ConcreteEventFunctorHandler
>
class EventBase {

	typedef std::list<AbstractEventHandler*> Handlers;
	typedef EventBase<AbstractEventHandler, ConcreteEventHandler, ConcreteEventFunctorHandler> EventClass;

	class AnonymousConnection {
	public:
		AbstractEventHandler* h_;
		EventClass* e_;
		AnonymousConnection(AbstractEventHandler* h, EventClass* e)
			: h_(h), e_(e)
		{
		}
	};

public:

	class Connection {
		AbstractEventHandler* h_;
		EventClass* e_;
	public:
		Connection()
			: h_(0), e_(0)
		{
		}

		Connection(AbstractEventHandler* h, EventClass* e)
			: h_(h), e_(e)
		{
		}

//		Connection(AnonymousConnection& c)
		Connection(AnonymousConnection c)
		{
			h_ = c.h_;
			e_ = c.e_;

			c.h_ = 0;
			c.e_ = 0;

			h_->ownerIsEvent(false);
		}

//		Connection& operator=(AnonymousConnection& c)
		Connection& operator=(AnonymousConnection c)
		{
			h_ = c.h_;
			e_ = c.e_;

			c.h_ = 0;
			c.e_ = 0;

			h_->ownerIsEvent(false);

			return *this;
		}

		void disconnect()
		{
			if (e_ && h_->eventIsAlive())
				e_->handlers_.remove(h_);

			delete h_;


			h_ = 0;
			e_ = 0;
		}

		~Connection()
		{
			disconnect();
		}
	private:
		Connection& operator=(Connection& c);
		Connection(Connection& c);
	};

	template <class Target, class MF>
	AnonymousConnection addListener(Target* target, MF mf)
	{
		AbstractEventHandler* h = new ConcreteEventHandler<Target, MF>(target, mf);
		handlers_.push_back(h);

		return AnonymousConnection(h, this);
	}

	template <class Functor>
	AnonymousConnection addListener(const Functor& f)
	{
		AbstractEventHandler* h = new ConcreteEventFunctorHandler<Functor>(const_cast<Functor&>(f));
		// 上面这行的const_cast是为了对付lambda
		handlers_.push_back(h);

		return AnonymousConnection(h, this);
	}

	~EventBase()
	{
		for (typename Handlers::iterator i = handlers_.begin(); i != handlers_.end(); ++i) {
//		for (Handlers::iterator i = handlers_.begin(); i != handlers_.end(); ++i) {
			if ((*i)->ownerIsEvent())
				delete *i;
			else
				(*i)->eventIsAlive(false);
		}
	}
protected:
	Handlers handlers_;
};

}	// namespace Shutters


#define EVENT_TYPE(event_name, parameters_list, arguments_list)	\
class Abstract##event_name##Handler : public Shutters::AbstractEventHandlerBase { \
public: \
	virtual void execute parameters_list = 0; \
}; \
\
template <class Target, class MF> \
class event_name##Handler : public Shutters::EventHandlerBase<Target, MF, Abstract##event_name##Handler> { \
	typedef Shutters::EventHandlerBase<Target, MF, Abstract##event_name##Handler> Base; \
public: \
	event_name##Handler(Target* target, MF mf) \
		: Base(target, mf) \
	{ \
	} \
	void execute parameters_list \
	{ \
		/*(target_->*mf_) arguments_list;*/ \
		(Base::target_->*Base::mf_) arguments_list; \
	} \
}; \
\
template <class Functor> \
class event_name##FunctorHandler : public Shutters::EventFunctorHandlerBase<Functor, Abstract##event_name##Handler> { \
	typedef Shutters::EventFunctorHandlerBase<Functor, Abstract##event_name##Handler> Base; \
public: \
	event_name##FunctorHandler(Functor& f) \
		: Base(f) \
	{ \
	} \
	void execute parameters_list \
	{ \
		/*f_ arguments_list;*/ \
		Base::f_ arguments_list; \
	} \
}; \
\
class event_name : public Shutters::EventBase<Abstract##event_name##Handler, event_name##Handler, event_name##FunctorHandler> { \
public: \
	void fire parameters_list \
	{ \
		for (Handlers::iterator i = handlers_.begin(); i != handlers_.end(); ++i) \
			(*i)->execute arguments_list; \
	} \
	bool existTarget() \
	{ \
		return !handlers_.empty(); \
	} \
};



#endif
