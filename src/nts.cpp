#include "nts.hpp"
#include <stdio.h>
#include <stdexcept>
#include <algorithm>  // find()
#include <limits>     // numeric_limits::max<T>()
#include <utility>    // move()
#include <iterator>   // distance()

#include "logic.hpp"
#include "to_csv.hpp"
#include "FilterIterator.hpp"
#include "TransformIterator.hpp"

using namespace nts;
using std::move;
using std::string;
using std::list;
using std::to_string;
using std::numeric_limits;
using std::find;
using std::ostream;
using std::distance;
using std::unique_ptr;
using std::function;
using std::pair;
using std::transform;

//------------------------------------//
// Annotations                        //
//------------------------------------//

Annotations::Annotations ( const Annotations & orig ) :
	list < Annotation * > {}
{
	for ( Annotation * a : orig )
	{
		Annotation * cl = a->clone();
		cl->insert_to ( *this );
	}
}

Annotations::~Annotations()
{
	for ( const Annotation * a : *this )
	{
		delete a;
	}
}

Annotations & Annotations::operator= ( const Annotations & orig )
{
	// Delete what we have
	for ( Annotation * a : *this )
		delete a;

	this->clear();

	for ( const Annotation * a : orig )
	{
		Annotation * cl = a->clone();
		cl->insert_to ( *this );
	}

	return *this;
}

void Annotations::print ( ostream & o ) const
{
	for ( const Annotation * a : *this )
	{
		o << *a << "\n";
	}
}

//------------------------------------//
// Nts                                //
//------------------------------------//

Nts::Nts ( string  name ) :
	initial_formula ( nullptr ),
	name  ( move ( name ) )
{
	;
}

Nts::~Nts()
{
	// Destroy them in the reverse order.
	// In this case, order of declarations
	// really does not matter - but better be sure ;)
	// Must be destoyed before  variables etc..
	this->initial_formula.reset();

	for ( auto i : _instances )
		delete i;
	_instances.clear();

	for ( auto b : _basics )
		delete b;
	_basics.clear();

}

void Nts::initial_add_conjunct ( unique_ptr < Formula > f )
{
	std::unique_ptr < Formula > new_formula;

	if ( initial_formula )
	{
		new_formula = std::make_unique < FormulaBop > (
				BoolOp::And,
				move ( f ),
				move ( initial_formula )
		);
	}
	else
	{
		new_formula = move ( f );
	}

	initial_formula = move ( new_formula );
	initial_formula->_parent_ptr.nts = this;
	initial_formula->_parent_type = Formula::ParentType::NtsInitialFormula;
}

unsigned int Nts::n_threads() const
{
	return std::accumulate (
			_instances.cbegin(),
			_instances.cend(),
			0,
			[] ( unsigned int n, const Instance * i )
			{ return n + i->num().evaluate(); }
	);
}

ostream & nts::operator<< ( ostream & o , const Nts & nts )
{
	o << "nts " << nts.name << ";\n";

#if 0
	if ( nts._pars.size() > 0 )
	{
		o << "par ";
		to_csv ( o, nts._pars.cbegin(), nts._pars.cend(),
				ptr_print_function<Variable>, "\n" ) << "\n";
	}
#endif

	if ( nts._vars.size() > 0 )
	{
		to_csv ( o, nts._vars.cbegin(), nts._vars.cend(),
				ptr_print_function<Variable>, "\n" ) << "\n";
	}

	if ( nts.initial_formula )
	{
		o << "init\t" << *nts.initial_formula << ";\n";
	}

	if ( nts._instances.size() > 0 )
	{
		o << "instances ";
		to_csv ( o, nts._instances.cbegin(), nts._instances.cend(),
				ptr_print_function<Instance>, ", " ) << ";\n";
	}

	if ( nts._basics.size() > 0 )
	{
		to_csv ( o, nts._basics.cbegin(), nts._basics.cend(),
				ptr_print_function<BasicNts>, "\n" ) << "\n";
	}

	return o;
}

//------------------------------------//
// Instance                           //
//------------------------------------//

Instance::Instance ( BasicNts *basic, Term* n )  :
	_parent ( nullptr ),
	_bn     ( basic   ),
  _n      ( n       )
{
  ;
}

Instance::~Instance()
{
  delete _n;
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

void Instance::insert_to ( Nts & parent )
{
	_parent = & parent;
	_pos = _parent->_instances.insert ( _parent->_instances.end(), this );
}

ostream & nts::operator<< ( ostream &o , const Instance &i )
{
	o << i._bn->name << '[' << *i._n << ']';
	return o;
}

//------------------------------------//
// State                              //
//------------------------------------//

State::State ( string name ) :
	_parent   ( nullptr       ),
	_initial  ( false         ),
	_final    ( false         ),
	_error    ( false         ),
	name      ( move ( name ) ),
	user_data ( nullptr       )
{
	;
}

bool State::operator== ( const State & s ) const
{
	return name == s.name;
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

void State::insert_after ( const State & s )
{
	if ( _parent )
		throw std::logic_error ( "State already belongs to BasicNts" );

	_parent = s._parent;
	auto where = s._pos;
	++where;
	_pos = _parent->_states.insert ( where, this );
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
	s.annotations.print ( o );
	o << "\t" << s.name;
	return o;
}

//------------------------------------//
// BasicNts                           //
//------------------------------------//
BasicNts::BasicNts ( string name ) :
	_parent   ( nullptr       ),
	name      ( move ( name ) ),
	user_data ( nullptr       )
{
	;
}

BasicNts::~BasicNts()
{
	// Deletion of transition unlinks it from the list,
	// so we can not call ++ on the iterator
	for ( auto t = _transitions.begin(); t != _transitions.end(); )
	{
		auto cur = t++;
		delete *cur;
	}
	// Now _transitions should be empty

	for ( auto s : _states )
		delete s;
	_states.clear();
}


void BasicNts::insert_to ( Nts & parent )
{
	_parent = & parent;
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

void BasicNts::print_params_in ( std::ostream & o ) const
{
	if ( _params_in.size() > 0 )
	{
		o << "\tin\t";

		to_csv( o,
				_params_in.cbegin(),
				_params_in.cend(),
				ptr_print_function < Variable >,
				",\n\t\t" );

		o << ";\n";
	}
}

void BasicNts::print_params_out ( std::ostream & o ) const
{
	if ( _params_out.size() > 0 )
	{
		o << "\tout\t" ;
		to_csv ( o,
				_params_out.cbegin(),
				_params_out.cend(),
				ptr_print_function < Variable >,
				",\n\t\t" );
		o << ";\n";
	}
}

void BasicNts::print_variables ( std::ostream & o ) const
{
	for ( auto v : _variables )
	{
		o << "\t" << *v << ";\n";
	}
}

template < typename T >
void ptr_name_print_function ( ostream & o, T * x )
{
	o << x->name;
}

template < typename InputIterator >
void print_state_list (
		ostream      & o,
		InputIterator  begin,
		InputIterator  end,
		const string & prefix,
		function < bool ( typename InputIterator::value_type ) > predicate,
		bool with_annotations = false
)
{
	Filtered < InputIterator > fs ( begin, end, predicate );
	auto b = fs.begin();
	auto e = fs.end();

	if ( distance ( b, e ) <= 0 )
		return;
	
	o << prefix;
	if ( with_annotations )
		to_csv ( o, b, e, ptr_print_function < State >, ",\n" );
	else
		to_csv ( o, b, e, ptr_name_print_function < State >, ", " );
	o << ";\n";
}

void BasicNts::print_states_basic ( std::ostream & o ) const
{
	auto p = [] ( State *s ) -> bool
	{
		// Print annotated states
		if ( s->annotations.size() > 0 )
			return true;

		if ( s->outgoing().size() >= 1 )
			return false;

		if ( s->incoming().size() >= 1 )
			return false;

		if ( s->is_initial() )
			return false;

		if ( s->is_final() )
			return false;

		return true;
	};

	print_state_list ( o, _states.cbegin(), _states.cend(), "\tstates\n", p, true );
}

void BasicNts::print_states_initial ( std::ostream & o ) const
{
	auto p = [] ( const State * s )
	{
		return s->is_initial();
	};

	print_state_list ( o, _states.cbegin(), _states.cend(), "\tinitial\t", p );
}

void BasicNts::print_states_final ( std::ostream & o ) const
{
	auto p = [] ( const State * s )
	{
		return s->is_final();
	};

	print_state_list ( o, _states.cbegin(), _states.cend(), "\tfinal\t", p );
}

void BasicNts::print_states_error ( std::ostream & o ) const
{
	auto p = [] ( const State * s )
	{
		return s->is_error();
	};

	print_state_list ( o, _states.cbegin(), _states.cend(), "\terror\t", p );
}

void BasicNts::print_transitions ( std::ostream & o ) const
{
	for ( auto t : _transitions )
	{
		o << "\t" << *t << "\n";
	}
}

std::ostream & nts::operator<< ( std::ostream &o, const BasicNts &bn)
{
	bn.annotations.print ( o );
	o << bn.name << " {\n";

	bn.print_params_in  ( o );
	bn.print_params_out ( o );
	bn.print_variables  ( o );

	bn.print_states_basic   ( o );
	bn.print_states_initial ( o );
	bn.print_states_final   ( o );
	bn.print_states_error   ( o );

	bn.print_transitions ( o );

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
				if ( & ctr.dest() == this->_callee)
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

Transition::Transition ( unique_ptr<TransitionRule> rule, State &s1, State &s2 ) :
	_parent   ( nullptr ),
	_from     ( s1      ),
	_to       ( s2      ),
	user_data ( nullptr )
{
	_rule = move ( rule );
	_rule->_t = this;

	_st_from_pos = _from._outgoing_tr.insert ( _from._outgoing_tr.cend(), this );
	_st_to_pos   = _to._incoming_tr.insert ( _to._incoming_tr.cend(), this );
}

Transition::~Transition()
{
	_from._outgoing_tr.erase ( _st_from_pos );
	_to._incoming_tr.erase ( _st_to_pos );

	if ( _parent )
	{
		_parent->_transitions.erase ( _pos );
		_parent = nullptr;
	}
}

void Transition::insert_to ( BasicNts & bn )
{
	if ( _parent )
		throw std::logic_error ( "Transition already has a parent" );

	if ( ( & bn != _from._parent ) || ( & bn != _to._parent ) )
		throw std::logic_error ( "States must belong to given BasicNts" );

	_parent = & bn;
	_pos    = _parent->_transitions.insert ( _parent->_transitions.cend(), this );
}

void Transition::remove_from_parent ()
{
	if ( ! _parent )
		throw std::logic_error ( "Transition does not have a parent" );

	_parent->_transitions.erase ( _pos );
	_parent = nullptr;
}

ostream & nts::operator<< ( ostream &o, const Transition &t )
{
	t.annotations.print ( o );
	o << t._from.name << " -> " << t._to.name << " " << *t._rule;
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

FormulaTransitionRule::FormulaTransitionRule ( unique_ptr<Formula> f ) :
	TransitionRule ( Kind::Formula ),
	_f             ( move ( f )    )
{
	set_formula_parent();
}

FormulaTransitionRule::FormulaTransitionRule ( const FormulaTransitionRule & orig ) :
	TransitionRule ( Kind::Formula )
{
	_f = unique_ptr < Formula > ( orig._f->clone() );
	set_formula_parent();
}

void FormulaTransitionRule::set_formula_parent()
{
	_f->_parent_ptr.ftr = this;
	_f->_parent_type = Formula::ParentType::FormulaTransitionRule;
}

ostream & FormulaTransitionRule::print ( std::ostream & o ) const
{
	o << "{ " << *this->_f << " }";
	return o;
}

FormulaTransitionRule * FormulaTransitionRule::clone() const
{
	return new FormulaTransitionRule ( *this );
}

//------------------------------------//
// CallTransitionRule                 //
//------------------------------------//

// 
template < typename It_1, typename It_2 >
bool CallTransitionRule::coercible (
				It_1 from_begin, It_1 from_end,
				It_2 to_begin,   It_2 to_end    )
{
	auto from = from_begin;
	auto to   = to_begin;
	for ( ;	from != from_end && to != to_end; ++from, ++to )
	{
		if ( ! coercible_ne ( *from, *to ) )
			return false;
	}

	if ( from != from_end || to != to_end )
		return false;
	
	return true;
}

template < typename Cont_1, typename Cont_2 >
bool CallTransitionRule::coercible ( const Cont_1 & from, const Cont_2 & to )
{
	return coercible
	<
		typename Cont_1::const_iterator,
		typename Cont_2::const_iterator
	> ( from.cbegin(), from.cend(), to.cbegin(), to.cend() );
}

bool CallTransitionRule::check_args (
				const BasicNts  & dest,
				const Terms     & in,
				const Variables & out  )
{
	const auto & par_in  = dest.params_in();
	const auto & par_out = dest.params_out();

	auto pterm_to_type = [] ( const Term *t ) -> const DataType &
	{
		return t->type();
	};

	auto pvar_to_type = [] ( const Variable * v ) -> const DataType &
	{
		return v->type();
	};

	auto caller_in  = Mapped < Terms    ::const_iterator, const DataType & >
		( in .cbegin(), in .cend(), pterm_to_type );
	auto caller_out = Mapped < Variables::const_iterator, const DataType & >
		( out.cbegin(), out.cend(), pvar_to_type  );

	auto callee_in  = Mapped < list<Variable *>::const_iterator, const DataType & >
		( par_in .cbegin(), par_in .cend(), pvar_to_type );
	auto callee_out = Mapped < list<Variable *>::const_iterator, const DataType & >
		( par_out.cbegin(), par_out.cend(), pvar_to_type );

	if ( ! coercible ( caller_in, callee_in ) )
		return false;

	if ( ! coercible ( callee_out, caller_out ) )
		return false;

	return true;
}


CallTransitionRule::CallTransitionRule ( BasicNts & dest, Terms in, Variables out ) :
	TransitionRule ( Kind::Call ),
	_dest          ( dest       ),
	_var_out       ( *this      )
{
	if ( !check_args ( dest, in, out ) )
		throw TypeError();

	_term_in = move ( in  );

	for ( Variable * v : out )
		_var_out.push_back ( v );

	set_terms_parent();
}

CallTransitionRule::CallTransitionRule ( const CallTransitionRule & orig ) :
	TransitionRule ( Kind::Call    ),
	_dest          ( orig._dest    ),
	_var_out       ( *this )
{
	_term_in.reserve ( orig._term_in.size() );
	_var_out = orig._var_out;

	for ( const Term * t : orig._term_in )
		_term_in.push_back ( t->clone() );

	set_terms_parent();
}


CallTransitionRule::~CallTransitionRule()
{
	while ( ! _term_in.empty() )
	{
		delete _term_in.back();
		_term_in.pop_back();
	}
}

CallTransitionRule * CallTransitionRule::clone() const
{
	return new CallTransitionRule ( *this );
}

void CallTransitionRule::transform_return_variables ( VarTransFunc f )
{
	transform (
			_var_out.cbegin(),
			_var_out.cend(),
			_var_out.begin(),
			[ &f ] ( const VariableUse & v1 ) -> Variable *
			{
				// TODO: Check type
				return f ( v1.get() );
			}
	);
}

void CallTransitionRule::set_terms_parent()
{
	for ( Term * t : _term_in )
	{
		t->_parent_type = Term::ParentType::CallTransitionRule;
		t->_parent_ptr.crule = this;
	}
}

namespace
{
	ostream & print_variable_name ( ostream &o, const VariableUse & v )
	{
		o << v->name;
		return o;
	}
};

ostream & CallTransitionRule::print ( std::ostream & o ) const
{
	o << "{ ";

	if ( _var_out.size() > 0 )
	{
		if ( _var_out.size() > 1 )
			o << "( ";

		to_csv ( o,
				_var_out.cbegin(),
				_var_out.cend(),
				print_variable_name, "', " ) << "'";

		if ( _var_out.size() > 1 )
			o << " )";

		o << " = ";
	}

	o << _dest.name << " ( ";
	to_csv ( o, _term_in.cbegin(), _term_in.cend(), ptr_print_function<Term>, ", " );
	o << " ) }";

	return o;
}

//------------------------------------//
// Variable                           //
//------------------------------------//

Variable::Variable ( DataType t, string name ) :
	_type      ( move ( t    ) ),
	_container ( nullptr       ),
	name       ( move ( name ) ),
	user_data  ( nullptr       )
{

}

Variable::Variable ( const Variable & orig ) :
	_type       ( orig._type       ),
	_container  ( nullptr          ),
	annotations ( orig.annotations ),
	name        ( orig.name        ),
	user_data   ( nullptr          )
{
	;
}

Variable::Variable ( const Variable && old ) :
	_type       ( move ( old._type       ) ),
	_container  ( move ( old._container  ) ),
	annotations ( move ( old.annotations ) ),
	name        ( move ( old.name        ) ),
	user_data   ( move ( old.user_data   ) )
{
	if ( _container )
	{
		_pos = old._pos;
		*(_pos) = this;
	}
}

void Variable::insert_to (
		VariableContainer                 & container,
		const VariableContainer::iterator & before    )
{
	if ( _container )
		throw std::logic_error ( "Variable already in container" );

	_container = & container;
	_pos = _container->insert ( before, this );
}

void Variable::remove_from_parent()
{
	if ( ! _container )
		throw std::logic_error ( "Variable does not have a parent" );

	_container->erase ( _pos );
	_container = nullptr;
}

void Variable::insert_to ( Nts & n )
{
	insert_to ( n._vars, n._vars.end() );
}

void Variable::insert_to ( BasicNts & nb )
{
	insert_to ( nb._variables, nb._variables.end() );
}

void Variable::insert_par ( Nts & n )
{
	insert_to ( n._pars, n._pars.end() );
}

void Variable::insert_par ( BasicNts & bn )
{
	insert_to ( bn._pars, bn._pars.end() );
}

void Variable::insert_param_in_to ( BasicNts & nb )
{
	insert_to ( nb._params_in, nb._params_in.end() );
}

void Variable::insert_param_out_to ( BasicNts & nb )
{
	insert_to ( nb._params_out, nb._params_out.end() );
}

void Variable::insert_to ( QuantifiedVariableList & ql )
{
	if ( ql.qtype().type() != _type )
		throw TypeError();

	insert_to ( ql._vars, ql._vars.end() );
}

void Variable::insert_before ( const Variable & var )
{
	if ( !var._container )
		throw std::logic_error ( "Variable does not have a parent" );

	insert_to ( *var._container, var._pos );
}

Variable * Variable::clone() const
{
	return new Variable ( *this );
}

ostream & nts::operator<< ( std::ostream & o, const Variable &v )
{
	v.annotations.print ( o );
	o << v.name;
	v._type.print_arr ( o );
	o <<  " : ";
	v._type.scalar_type().print ( o );
	return o;
}

//------------------------------------//
// BitVector Variable                 //
//------------------------------------//

BitVectorVariable::BitVectorVariable ( string name, unsigned int width ) :
	Variable ( DataType ( ScalarType::BitVector(width) ), move ( name ) )
{
	;
}

//------------------------------------//
// Annotation                         //
//------------------------------------//

Annotation::Annotation ( string name, Type t ) :
	_type   ( move ( t    ) ),
	_parent ( nullptr       ),
	name    ( move ( name ) )
{
	;
}

void Annotation::insert_to ( Annotations & ants )
{
	if ( _parent )
		throw std::logic_error ( "Already have a parent" );

	_parent = & ants;
	_pos = _parent->insert ( _parent->cend(), this );
}

void Annotation::remove_from_parent()
{
	if ( !_parent )
		throw std::logic_error ( "Does not have a parent" );

	_parent->erase ( _pos );
	_parent = nullptr;
}

ostream & nts::operator<< ( ostream & o, const Annotation & a )
{
	o << "@" << a.name << ":";
	a.print ( o );
	o << ";";
	return o;
}

//------------------------------------//
// AnnotString                        //
//------------------------------------//

AnnotString::AnnotString ( string name, string value ) :
	Annotation ( move ( name ), Type::String ),
	value      ( move ( value ) )
{
	;
}

AnnotString::AnnotString ( const AnnotString & orig ) :
	Annotation ( orig.name, orig.type() ),
	value      ( orig.value )
{
	;
}

void AnnotString::print ( ostream & o ) const
{
	o << "string:\"" << value << "\"";
}

AnnotString * AnnotString::clone() const
{
	return new AnnotString ( *this );
}


