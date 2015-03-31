#include "nts.hpp"
#include <stdio.h>

void instance::remove_from_parent()
{
	if ( _parent )
	{
		_parent->_instances.erase ( _pos );
		_parent = nullptr;
	}
}

void instance::insert_before ( const instance &i )
{
	_parent = i._parent;
	_pos = _parent->_instances.insert ( i._pos, this );
}

void instance::insert_to_nts ( nts * parent )
{
	_parent = parent;
	_pos = _parent->_instances.insert ( _parent->_instances.end(), this );
}

void nts_basic::insert_to_nts ( nts * parent )
{
	_parent = parent;
	_pos = _parent->_basics.insert ( _parent->_basics.end(), this );
}

void nts_basic::remove_from_parent()
{
	if ( _parent ) 
	{
		_parent->_basics.erase ( _pos );
		_parent = nullptr;
	}
}

nts_basic::Callees nts_basic::callees()
{
	return Callees ( _transitions );
}

nts_basic::Callers nts_basic::callers()
{
	return Callers ( this, _parent->_basics );
}

void transition::insert_to_nts_basic ( nts_basic * parent )
{
	_parent = parent;
	_pos = _parent->_transitions.insert ( _parent->_transitions.end(), this );
}

void transition::remove_from_parent()
{
	if ( _parent )
	{
		_parent->_transitions.erase ( _pos );
		_parent = nullptr;
	}
}

bool transition::has_call_rule() const
{
	return _call_rule != nullptr;
}

struct Example
{
	call_rule cr[2];
	formula_rule fr[2];
	transition tr[4];

	nts_basic nb[2];
	nts toplevel_nts;

	Example() :
		cr { call_rule ( &nb[1] ),
			 call_rule ( &nb[1] )},
		tr { transition ( &fr[0] ),
		     transition ( &cr[0] ),
		     transition ( &cr[1] ),
		     transition ( &fr[1] )}
	{
		for ( int i = 0; i < 4; i++ )
			tr[i].insert_to_nts_basic ( &nb[0] );

		nb[0].insert_to_nts ( &toplevel_nts );
		nb[1].insert_to_nts ( &toplevel_nts );
	}

	void try_callees()
	{
		auto i = nb[0].callees().begin();
		printf ( "%p == %p\n", &tr[1], *i);
		i++;
		printf ( "%p == %p\n", &tr[2], *i);
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
