#include "nts.hpp"
#include <stdio.h>

using namespace nts;

void Instance::remove_from_parent()
{
	if ( _parent )
	{
		_parent->_Instances.erase ( _pos );
		_parent = nullptr;
	}
}

void Instance::insert_before ( const Instance &i )
{
	_parent = i._parent;
	_pos = _parent->_Instances.insert ( i._pos, this );
}

void Instance::insert_to_nts ( Nts * parent )
{
	_parent = parent;
	_pos = _parent->_Instances.insert ( _parent->_Instances.end(), this );
}

void BasicNts::insert_to_nts ( Nts * parent )
{
	_parent = parent;
	_pos = _parent->_basics.insert ( _parent->_basics.end(), this );
}

void BasicNts::remove_from_parent()
{
	if ( _parent ) 
	{
		_parent->_basics.erase ( _pos );
		_parent = nullptr;
	}
}

BasicNts::Callees BasicNts::callees()
{
	return Callees ( _Transitions );
}

BasicNts::Callers BasicNts::callers()
{
	return Callers ( this, _parent->_basics );
}

void Transition::insert_to_BasicNts ( BasicNts * parent )
{
	_parent = parent;
	_pos = _parent->_Transitions.insert ( _parent->_Transitions.end(), this );
}

void Transition::remove_from_parent()
{
	if ( _parent )
	{
		_parent->_Transitions.erase ( _pos );
		_parent = nullptr;
	}
}

struct Example
{
	CallTransition ct[2];
	Transition tr[2];

	BasicNts nb[2];
	Nts toplevel_nts;

	Example() :
		ct {
			CallTransition ( &nb[1] ),
			CallTransition ( &nb[1] )
		},
		tr {
			Transition ( Transition::Kind::Formula ),
			Transition ( Transition::Kind::Formula )
		}
	{
		tr[0].insert_to_BasicNts ( &nb[0] );
		ct[0].insert_to_BasicNts ( &nb[0] );
		ct[1].insert_to_BasicNts ( &nb[0] );
		tr[1].insert_to_BasicNts ( &nb[0] );


		nb[0].insert_to_nts ( &toplevel_nts );
		nb[1].insert_to_nts ( &toplevel_nts );
	}

	void try_callees()
	{
		auto i = nb[0].callees().begin();
		printf ( "%p == %p\n", &ct[0], *i);
		i++;
		printf ( "%p == %p\n", &ct[1], *i);
		i++;
		printf ( "end? %s\n", i == nb[0].callees().end() ? "yes" : "no" );
	}

	void try_callers()
	{
		auto i = nb[1].callers().begin();
		auto e = nb[1].callers().end();
		for (int j = 0; j < 5 && i != e; ++j, ++i )
		{
			printf( "%d: %p\n", j, (*i) );
		}
	}

};

int main ( void )
{
	printf ( "Hello world\n" );

	Example e1;
	e1.try_callees();
	e1.try_callers();



	return 0;
}
