#include "nts.hpp"
#include <stdio.h>
#include <stdexcept>
#include <utility> // move()

using namespace nts;

void Instance::remove_from_parent()
{
	if ( _parent )
	{
		_parent->_instances.erase ( _pos );
		_parent = nullptr;
	}
}

void Instance::insert_before ( const Instance &i )
{
	_parent = i._parent;
	_pos = _parent->_instances.insert ( i._pos, this );
}

void Instance::insert_to ( Nts * parent )
{
	_parent = parent;
	_pos = _parent->_instances.insert ( _parent->_instances.end(), this );
}

void BasicNts::insert_to ( Nts * parent )
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
	return Callees ( _transitions );
}

BasicNts::Callers BasicNts::callers()
{
	return Callers ( this, _parent->_basics );
}

void Transition::insert_to ( BasicNts * parent )
{
	_parent = parent;
	_pos = _parent->_transitions.insert ( _parent->_transitions.end(), this );
}

void Transition::remove_from_parent()
{
	if ( _parent )
	{
		_parent->_transitions.erase ( _pos );
		_parent = nullptr;
	}
}

//------------------------------------//
// Variable                           //
//------------------------------------//

Variable::Variable ( Type t, const std::string & name ) :
	_type        ( t       ),
	_name        ( name    ),
	_parent_list ( nullptr )
{

}

Variable::Variable ( const Variable && old ) :
	_type ( std::move ( old._type ) ),
	_name ( std::move ( old._name ) ),
	_parent_list ( old._parent_list )
{
	if ( _parent_list )
	{
		_pos = old._pos;
		*(_pos) = this;
	}
}

void Variable::insert_to ( Variables *parent, const Variables::iterator & before )
{
	if ( _parent_list )
		throw std::logic_error ( "Variable already has a parent" );

	_parent_list = parent;
	_pos = _parent_list->insert ( before, this );
}

void Variable::remove_from_parent()
{
	if ( ! _parent_list )
		throw std::logic_error ( "Variable does not have a parent" );

	_parent_list->erase ( _pos );
	_parent_list = nullptr;
}

void Variable::insert_to ( Nts * n )
{
	insert_to ( & n->_vars, n->_vars.end() );
}

void Variable::insert_to ( BasicNts *nb )
{
	insert_to ( & nb->_variables, nb->_variables.end() );
}

void Variable::insert_param_in_to ( BasicNts *nb )
{
	insert_to ( & nb->_params_in, nb->_params_in.end() );
}

void Variable::insert_param_out_to ( BasicNts *nb )
{
	insert_to ( & nb->_params_out, nb->_params_out.end() );
}

void Variable::insert_before ( const Variable & var )
{
	if ( !var._parent_list )
		throw std::logic_error ( "Variable does not have a parent" );

	insert_to ( var._parent_list, var._pos );
}

//------------------------------------//
// BitVector Variable                 //
//------------------------------------//

BitVectorVariable::BitVectorVariable ( const std::string &name, unsigned int width ) :
	Variable ( Variable::Type::BitVector, name ),
	_bitwidth ( width )
{
	;
}

struct Example
{
	CallTransition ct[2];
	Transition tr[2];

	BitVectorVariable bvvar[4];

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
		},
		bvvar {
			BitVectorVariable ( "var1", 8 ),
			BitVectorVariable ( "var2", 16),
			BitVectorVariable ( "var3", 1 ),
			BitVectorVariable ( "var4", 4 )
		}
	{
		tr[0].insert_to ( &nb[0] );
		ct[0].insert_to ( &nb[0] );
		ct[1].insert_to ( &nb[0] );
		tr[1].insert_to ( &nb[0] );


		nb[0].insert_to ( &toplevel_nts );
		nb[1].insert_to ( &toplevel_nts );

		bvvar[0].insert_to ( & toplevel_nts );
		bvvar[1].insert_param_in_to ( & nb[1] );
		bvvar[2].insert_before ( bvvar[1] );

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
