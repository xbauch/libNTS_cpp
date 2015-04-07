#include "nts.hpp"
#include <stdio.h>
#include <stdexcept>
#include <algorithm>  // find()
#include <limits>     // numeric_limits::max<T>()
#include <utility>    // move()
#include <iterator>   // distance()

#include "to_csv.hpp"
#include "FilterIterator.hpp"

using namespace nts;
using std::string;
using std::list;
using std::to_string;
using std::numeric_limits;
using std::find;
using std::ostream;
using std::distance;

//------------------------------------//
// Nts                                //
//------------------------------------//

Nts::Nts ( const string & name ) :
	_name ( name )
{
	;
}


Nts::~Nts()
{
	for ( auto i : _instances )
		delete i;
	_instances.clear();

	for ( auto b : _basics )
		delete b;
	_basics.clear();

	for ( auto v : _vars )
		delete v;
	_vars.clear();
}


//------------------------------------//
// Instance                           //
//------------------------------------//

Instance::Instance ( BasicNts *basic, unsigned int n )  :
	_parent  ( nullptr ),
	function ( basic   ),
	n        ( n       )
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

ostream & nts::operator<< ( ostream &o , const Instance &i )
{
	o << i.function->name() << '[' << i.n << ']';
	return o;
}

//------------------------------------//
// State                              //
//------------------------------------//

State::State ( const std::string & name ) :
	_parent ( nullptr ),
	_name   ( name    )
{
	;
}

State::State ( const std::string && name ) :
	_parent ( nullptr ),
	_name   ( name    )
{
	;
}


State::State ( const State && old ) :
	_name ( std::move ( old._name ) )
{
	;
}

bool State::operator== ( const State & s ) const
{
	return _name == s._name;
}

bool State::operator!= ( const State & s ) const
{
	return ! ( *this == s );
}

void State::insert_to ( BasicNts &n )
{
	if ( _parent )
		throw std::logic_error ( "State already belongs to BasicNts" );

	_parent = &n;
	_pos = n._states.insert ( n._states.cend(), this );
}

void State::remove_from_parent ()
{
	if ( !_parent )
		throw std::logic_error ( "State does not belong to any BasicNts" );

	_parent->_states.erase ( _pos );
	_parent = nullptr;
}

ostream & nts::operator<< ( ostream &o , const State &s )
{
	o << s._name;
	return o;
}

//------------------------------------//
// BasicNts                           //
//------------------------------------//
BasicNts::BasicNts ( const std::string & name ) :
	_name   ( name    ),
	_parent ( nullptr )
{
	;
}

BasicNts::~BasicNts()
{
	{
		// Deletion of transition unlinks it from the list,
		// so we can not call ++ on the iterator
		for ( auto t = _transitions.begin(); t != _transitions.end(); )
		{
			auto cur = t++;
			delete *cur;
		}

		// Now _transitions should be empty
	}

	for ( auto s : _states )
		delete s;
	_states.clear();

	for ( auto v : _variables )
		delete v;
	_variables.clear();

	for ( auto p : _params_in )
		delete p;
	_params_in.clear();

	for ( auto p : _params_out )
		delete p;
	_params_out.clear();
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

std::ostream & nts::operator<< ( std::ostream &o, const BasicNts &bn)
{
	// Prints variable
	auto var_pr = [] ( ostream &o, Variable *v )
	{
		o << *v;
	};

	auto st_pr = [] ( ostream &o, const State *s )
	{
		o << *s;
	};


	o << bn._name << " {\n";

	if ( bn._params_in.size() > 0 )
	{
		o << "\tin\t";
		to_csv( o, bn._params_in.cbegin(), bn._params_in.cend(), var_pr, ",\n\t\t" );
		o << ";\n";
	}

	if ( bn._params_out.size() > 0 )
	{
		o << "\tout\t";
		to_csv ( o, bn._params_out.cbegin(), bn._params_out.cend(), var_pr, ",\n\t\t" );
		o << ";\n";
	}

	for ( auto v : bn._variables )
	{
		o << "\t" << *v << ";\n";
	}

	auto filter_states = [] ( State *s ) -> bool
	{
		if ( s->outgoing().size() >= 1 )
			return false;

		if ( s->incoming().size() >= 1 )
			return false;

		return true;
	};

	// All states without incoming / outgoing edges
	Filtered < decltype ( bn._states)::const_iterator >
		fs ( bn._states.cbegin(), bn._states.cend(), filter_states );

	if ( distance ( fs.begin(), fs.end() )  >= 1 )
	{
		o << "\tstates\t";
		to_csv ( o, fs.begin(), fs.end(), st_pr, "\n\t\t" );
		o << ";\n";
	}

	for ( auto t : bn._transitions )
	{
		o << "\t" << *t << "\n";
	}

	o << "}\n";

	return o;
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

const CallTransitionRule & BasicNts::Callees::iterator::operator* () const
{
	return static_cast < const CallTransitionRule & > ( (*_it)->rule() );
}

void BasicNts::Callees::iterator::skip()
{
	while ( _it != _t.end() &&
			(*_it)->rule().kind() != TransitionRule::Kind::Call)
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
			if ( (*_Transition)->rule().kind() == TransitionRule::Kind::Call )
			{
				auto &ctr = static_cast< const CallTransitionRule &>
					( (*_Transition)->rule() );
				if ( ctr.dest() == this->_callee)
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

const CallTransitionRule & BasicNts::Callers::iterator::operator*() const
{
	return static_cast<const CallTransitionRule &>((*_Transition)->rule()) ;
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

Transition::Transition ( TransitionRule & rule, State &s1, State &s2 ) :
	_parent ( *s1._parent ),
	_rule   ( rule        ),
	_from   ( s1          ),
	_to     ( s2          )
{
	if ( !s1._parent || !s2._parent )
		throw std::logic_error ( "Transition states must have parents" );

	if ( s1._parent != s2._parent )
		throw std::logic_error ( "Transition states must have the same parents" );

	rule._t = this;

	_st_from_pos = _from._outgoing_tr.insert ( _from._outgoing_tr.cend(), this );
	_st_to_pos   = _to._incoming_tr.insert ( _to._incoming_tr.cend(), this );

	_pos = _parent._transitions.insert ( _parent._transitions.end(), this );
}

Transition::~Transition()
{
	_from._outgoing_tr.erase ( _st_from_pos );
	_to._incoming_tr.erase ( _st_to_pos );

	_parent._transitions.erase ( _pos );
	delete &_rule;
}

ostream & nts::operator<< ( ostream &o, const Transition &t )
{
	o << t._from << " -> " << t._to << " " << t._rule;
	return o;
}

//------------------------------------//
// TransitionRule                     //
//------------------------------------//

ostream & nts::operator<< ( ostream &o, const TransitionRule &tr )
{
	tr.print ( o );
	return o;
}

//------------------------------------//
// FormulaTransitionRule              //
//------------------------------------//

FormulaTransitionRule::FormulaTransitionRule ( const Formula *f ) :
	TransitionRule ( Kind::Formula ),
	_f             ( f             )
{
	;
}

ostream & FormulaTransitionRule::print ( std::ostream & o ) const
{
	// TODO implement
	o << "{}";
	return o;
}

//------------------------------------//
// CallTransitionRule                 //
//------------------------------------//

CallTransitionRule::CallTransitionRule
(
		BasicNts   * dest,
		VariableList in,
		VariableList out
) :
	TransitionRule ( Kind::Call ),
	_dest          ( dest       )
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

namespace
{
	ostream & print_variable_name ( ostream &o, const Variable * v )
	{
		o << v->name();
		return o;
	}
};

ostream & CallTransitionRule::print ( std::ostream & o ) const
{
	o << "{ ";

	if ( _var_out.size() > 1 )
		o << "( ";

	to_csv < decltype(_var_out)::const_iterator, print_variable_name >
		( o, _var_out.cbegin(), _var_out.cend() );

	if ( _var_out.size() > 1 )
		o << " )";

	o << " = " << _dest->name() << " ( ";
	to_csv < decltype(_var_out)::const_iterator, print_variable_name >
		( o, _var_in.cbegin(), _var_in.cend() );
	o << " ) }";

	return o;
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

ostream & nts::operator<< ( std::ostream & o, const Variable &v )
{
	o << v._name << " : ";
	v._type.print ( o );
	return o;
}

//------------------------------------//
// BitVector Variable                 //
//------------------------------------//

BitVectorVariable::BitVectorVariable ( const std::string &name, unsigned int width ) :
	Variable ( DataType::BitVector(width), name )
{
	;
}


