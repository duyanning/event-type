#include "pch.h"
#include "../EVENT_TYPE.hpp"

using namespace std;

class Edit {
	string s_;
public:
	// define event type 'TextChanged'
	EVENT_TYPE(TextChanged, (string s), (s));

	TextChanged changed;

	void set(string s)
	{
		s_ = s;

		// fire 'changed' event when the content of Edit is altered
		changed.fire(s_);
	}
};

class Dialog {
	Edit edit_;
public:
	void OnEditChanged(string s)
	{
		cout << "text in edit is : " << s << endl;
	}

	Dialog()
	{
		//edit_.changed.addListener(this, OnEditChanged);	// vc7.1
		edit_.changed.addListener(this, &Dialog::OnEditChanged);
	}

	void input(string s)
	{
		edit_.set(s);
	}

};


void example1()
{
	Dialog dlg;
	dlg.input("some text");
	dlg.input("another text");
}



EVENT_TYPE(SomeEvent, (int x, int y), (x, y));
SomeEvent event1;

// function object
class Foo {
public:
	void operator()(int a, int b)
	{
		cout << "function object: " << a << '+' << b << '=' << a + b << endl;
	}
};

// ordinary function
void bar(int a, int b)
{
	cout << "function: " << a << '+' << b << '=' << a + b << endl;
}

void example2()
{
	Foo foo1;
	// connect to a function object
	SomeEvent::Connection c1(event1.addListener(foo1));

	SomeEvent::Connection c2;
	// connect to an ordinary function
	c2 = event1.addListener(bar);

	//SomeEvent::Connection c3(c1);	// error: copy construction is not permitted

	//c1 = c2;	// error: assignment is not permitted

	event1.fire(1, 2);

	{
		Foo foo2;
		SomeEvent::Connection c4(event1.addListener(foo2));

		// create an anonymous connection
		event1.addListener(bar);

		event1.fire(3, 4);

		// c4 break up automaticly
	}

	c1.disconnect();
	event1.fire(5, 6);

}

// connection's lifetime longer than event, no problem!
void example3()
{
	SomeEvent::Connection c;

	{
		SomeEvent event2;
		c = event2.addListener(bar);
		event2.fire(9, 9);
	}
	// you can invoke c.disconnect() explicitly or
	// rely on c's destructor to do it
}

int main()
{
#ifdef _MSC_VER
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	example1();
	example2();
	example3();
}
