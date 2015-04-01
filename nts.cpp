#include "nts.hpp"
#include <stdio.h>
#include <stdexcept>
#include <utility> // move()

using namespace nts;

//------------------------------------//
// Instance                           //
//------------------------------------//

Instance::Instance ( BasicNts *basic, unsigned int n )  :
	function ( basic ), n ( n )
{
	;
}


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

//------------------------------------//
// BasicNts                           //
//------------------------------------//

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

//------------------------------------//
// BasicNts::Callees                  //
//------------------------------------//

BasicNts::Callees BasicNts::callees()
{
	return Callees ( _transitions );
}

BasicNts::Callees::iterator::iterator (
		const BasicNts::Callees::Transitions::iterator &it,
		const BasicNts::Callees::Transitions &t ) :
	_it ( it ),
	_t  ( t  )
{
	skip();
}

BasicNts::Callees::iterator & BasicNts::Callees::iterator::operator++ ()
{
	if ( _it != _t.end() )
	{
		_it++;
		skip();
	}
	return *this;
}

BasicNts::Callees::iterator BasicNts::Callees::iterator::operator++ ( int )
{
	iterator old ( *this );
	operator++();
	return old;
}

bool BasicNts::Callees::iterator::operator==
	( const BasicNts::Callees::iterator &rhs ) const
{
	return _it == rhs._it;
}

bool BasicNts::Callees::iterator::operator!=
	( const BasicNts::Callees::iterator &rhs ) const
{
	return *this != rhs;
}

Transition * & BasicNts::Callees::iterator::operator* ()
{
	return *_it;
}

void BasicNts::Callees::iterator::skip()
{
	while ( _it != _t.end() &&
			(*_it)->kind() != Transition::Kind::Call)
	{
			_it++;
	}
}

BasicNts::Callees::iterator BasicNts::Callees::begin()
{
	return iterator ( _transitions.begin(), _transitions );
}

BasicNts::Callees::iterator BasicNts::Callees::end()
{
	return iterator ( _transitions.end(), _transitions );
}

//------------------------------------//
// BasicNts::Callers                  //
//------------------------------------//

BasicNts::Callers BasicNts::callers()
{
	return Callers ( this, _parent->_basics );
}

BasicNts::Callers::iterator::iterator (
		const Basics::iterator & it,
		const Basics           & basics,
		const BasicNts        * callee ) :

	_basics    ( basics ),
	_callee    ( callee ),
	_BasicNts ( it )
{
	if ( _BasicNts != _basics.end() )
	{
		_Transition = (*_BasicNts)->_transitions.begin();
		skip();
	}
}

BasicNts::Callers::iterator BasicNts::Callers::begin()
{
	return iterator ( _basics.begin(), _basics, _callee );
}

BasicNts::Callers::iterator BasicNts::Callers::end()
{
	return iterator ( _basics.end(), _basics, _callee );
}

void BasicNts::Callers::iterator::skip()
{
	while ( _BasicNts != _basics.end() )
	{
		auto e = (*_BasicNts)->_transitions.end();
		while ( _Transition != e )
		{
			if ( (*_Transition)->kind() == Transition::Kind::Call )
			{
				const CallTransition *ct = (CallTransition *) (*_Transition);
				if ( ct->dest() == this->_callee)
					return;
			}
			_Transition++;
		}

		_BasicNts++;

		if ( _BasicNts != _basics.end() )
			_Transition = (*_BasicNts)->_transitions.begin();
	}
}

BasicNts::Callers::iterator & BasicNts::Callers::iterator::operator++()
{
	if ( _BasicNts != _basics.end() )
	{
		_Transition++;
		skip();
	}

	return *this;
}

BasicNts::Callers::iterator BasicNts::Callers::iterator::operator++(int)
{
	iterator old = *this;
	operator++();
	return old;
}

CallTransition * BasicNts::Callers::iterator::operator*()
{
	return (CallTransition *) (*_Transition);
}

bool BasicNts::Callers::iterator::operator== ( const iterator &rhs ) const
{
	return _BasicNts == rhs._BasicNts &&
		( _BasicNts == _basics.end() || _Transition == rhs._Transition);
}

bool BasicNts::Callers::iterator::operator!= ( const iterator &rhs) const
{
	return !(*this == rhs);
}



//------------------------------------//
// Transition                         //
//------------------------------------//

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

Transition::Transition ( Transition::Kind k ) :
	_kind ( k )
{
	;
}

Transition::Kind Transition::kind() const
{
	return _kind;
}

//------------------------------------//
// CallTransition                     //
//------------------------------------//

CallTransition::CallTransition ( BasicNts * dest, VariableList in, VariableList out ) :
	Transition ( Kind::Call ),
	_dest      ( dest       )
{
	const auto & params_in  = dest->params_in();
	const auto & params_out = dest->params_out();

	if ( in.size() != params_in.size() )
		throw TypeError();

	if ( out.size() != params_out.size() )
		throw TypeError();

	// Check type of input parameters
	auto formal = params_in.cbegin();
	for ( const auto i : in )
	{
		if ( formal == params_in.cend() )
			throw TypeError();

		if ( (*formal)->type() != i->type() )
			throw TypeError();

		formal++;
	}

	// Check type of output parameters
	formal = params_out.cbegin();
	for ( const auto i : out )
	{
		if ( formal == params_out.cend() )
			throw TypeError();

		if ( (*formal)->type() != i->type() )
			throw TypeError();

		formal++;
	}

	_var_in.insert ( _var_in.cbegin(), in );
	_var_out.insert ( _var_out.cbegin(), out );

}

//------------------------------------//
// Variable                           //
//------------------------------------//

Variable::Variable ( DataType t, const std::string & name ) :
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
	Variable ( DataType::BitVector(width), name )
{
	;
}


