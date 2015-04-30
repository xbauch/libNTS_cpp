#include <map>
#include <string>
#include <utility>
#include <unordered_set>
#include <algorithm>
#include <stdexcept>

#include "nts.hpp"
#include "logic.hpp"

using namespace nts;

using std::map;
using std::string;
using std::unordered_set;
using std::make_pair;
using std::find_if;
using std::logic_error;
using std::unique_ptr;
using std::to_string;

#if 0
class FunctionInliner
{
	private:
		BasicNts & _bn;
		map < string, Variable * > _visible_names;

	public:

		FunctionInliner ( BasicNts & bn );

		/**
		 * @pre nothing
		 * @post _visible_names contains all names visible from
		 * _bn (i.e. global variables, local variables and parameters)
		 */
		void init_names();
};

FunctionInliner::FunctionInliner ( BasicNts & bn ) :
	_bn ( bn )
{
	;
}

void FunctionInliner::init_names()
{
	_visible_names.clear();
	for ( Variable *v : _bn.params_in() )
		_visible_names.insert ( make_pair ( v->name, v ) );

	for ( Variable *v : _bn.params_out() )
		_visible_names.insert ( make_pair ( v->name, v ) );

	for ( Variable *v : _bn.variables() )
		_visible_names.insert ( make_pair ( v->name, v ) );

	if ( _bn.parent() )
		for ( Variable *v : _bn.parent()->variables() )
			_visible_names.insert ( make_pair ( v->name, v ) );
}
#endif

/*
 * T must provide member access to 'annotations ( list of annotations)'
 * and to 'std::string name() const'
 */
template < typename T >
void annotate_with_origin ( T & x )
{
	Annotation * a = new AnnotString ( "origin", x.name );
	a->insert_to ( x.annotations );
}

void annotate_with_origin ( BasicNts & bn )
{
	for ( Variable *v : bn.variables() )
		annotate_with_origin ( *v );

	for ( State *s : bn.states() )
		annotate_with_origin ( *s );
}


AnnotString * find_origin ( Annotations & ants )
{
	Annotations::iterator it = find_if ( ants.begin(), ants.end(), 
			[] ( const Annotation *a) -> bool
			{
				return a->name == "origin" && a->type() == Annotation::Type::String;
			}
	);

	if ( it == ants.end() )
		return nullptr;

	return static_cast < AnnotString * > ( *it );
}

void substitute_variables ( AtomicProposition & ap );
void substitute_variables ( Term & t );
void substitute_variables ( Formula & f );

unique_ptr < Term > substitute_term ( unique_ptr < Term > t );




Variable * substitute ( Variable * var )
{
	if ( var->user_data == nullptr )
		return var;

	Variable * v2 = ( Variable * ) var->user_data;
	if ( v2->type() != var->type() )
		throw TypeError();

	return v2;
}

const Variable * substitute_const ( const Variable * var )
{
	if ( var->user_data == nullptr )
		return var;

	Variable * v2 = ( Variable * ) var->user_data;
	if ( v2->type() != var->type() )
		throw TypeError();

	return v2;
}

//------------------------------------//
// Term                               //
//------------------------------------//

void substitute_variables ( Leaf & lf )
{
	if ( lf.leaf_type() == Leaf::LeafType::VariableReference )
	{
		VariableReference & vr = ( VariableReference & ) lf;
		if ( vr.variable().user_data )
			vr.substitute ( * ( Variable * )vr.variable().user_data );
	}
}

void substitute_variables ( MinusTerm & mt )
{
	substitute_variables ( mt.term() );
}

void substitute_variables ( ArrayTerm & art )
{
	art.transform_indices ( substitute_term );
}

void substitute_variables ( ArithmeticOperation & aop )
{
	substitute_variables ( aop.term1() );
	substitute_variables ( aop.term2() );
}

void substitute_variables ( Term & t )
{
	switch ( t.term_type() )
	{
		case Term::TermType::Leaf:
			return substitute_variables ( ( Leaf & ) t );

		case Term::TermType::MinusTerm:
			return substitute_variables ( ( MinusTerm & ) t );

		case Term::TermType::ArrayTerm:
			return substitute_variables ( ( ArrayTerm & ) t );

		case Term::TermType::ArithmeticOperation:
			return substitute_variables ( ( ArithmeticOperation & ) t );
	}
}

unique_ptr < Term > substitute_term ( unique_ptr < Term > t )
{
	substitute_variables ( *t );
	return t;
}

//------------------------------------//
// AtomicProposition                  //
//------------------------------------//

void substitute_variables ( Havoc & h )
{
	std::vector<Variable * > outv;
	std::transform ( h.variables.begin(),
			h.variables.end(),
			h.variables.begin(),
			substitute
	);
}

void substitute_variables ( ArrayWrite & aw )
{
	for ( Term * t : aw.indices_1() )
		substitute_variables ( *t );

	for ( Term * t : aw.indices_2() )
		substitute_variables ( *t );

	for ( Term * t : aw.values() )
		substitute_variables ( *t );
}

void substitute_variables ( Relation & r )
{
	substitute_variables ( r.term1() );
	substitute_variables ( r.term2() );
}

void substitute_variables ( BooleanTerm & bt )
{
	substitute_variables ( bt.term() );
}

//------------------------------------//
// Formula                            //
//------------------------------------//

void substitute_variables ( AtomicProposition & ap )
{
	switch ( ap.aptype() )
	{
		case AtomicProposition :: APType :: Relation:
			return substitute_variables ( ( Relation    & ) ap );

		case AtomicProposition :: APType :: Havoc:
			return substitute_variables ( ( Havoc       & ) ap );

		case AtomicProposition :: APType :: ArrayWrite:
			return substitute_variables ( ( ArrayWrite  & ) ap );

		case AtomicProposition :: APType :: BooleanTerm:
			return substitute_variables ( ( BooleanTerm & ) ap );
	}
	
	throw logic_error ( "Unknown APType" );
}

void substitute_variables ( FormulaNot & f )
{
	substitute_variables ( f.formula() );
}

void substitute_variables ( FormulaBop & fb )
{
	substitute_variables ( fb.formula_1() );
	substitute_variables ( fb.formula_2() );
}

void substitute_variables ( QuantifiedFormula & qf )
{
	throw logic_error ( "Not implemented" );
}




/**
 * @brief Substitute variables inside formula with their shadow variables.
 * @pre Each variable in formula must have 'user_data' set to nullptr
 * or pointing to another variable.
 * @post Occurences of variables, which had 'user data' pointing to some other variable,
 * are substituted with the other variable. Nothing else is modified.
 */
void substitute_variables ( Formula & f )
{
	switch ( f.type() )
	{
		case Formula::Type::AtomicProposition:
			return substitute_variables ( (AtomicProposition &) ( f ) );

		case Formula::Type::FormulaNot:
			return substitute_variables ( (FormulaNot & ) ( f ) );

		case Formula::Type::FormulaBop:
			return substitute_variables ( (FormulaBop & ) ( f ) );

		case Formula::Type::QuantifiedFormula:
			return substitute_variables ( (QuantifiedFormula & ) ( f ) );
	}

	throw logic_error ( "Unknown formula type" );
}

/**
 * @brief copies 'v' to 'bn' an makes v.user_pointer point to the copy
 * @pre v must have 'origin' annotation. @see annotate_variable()
 */
void transfer_to ( BasicNts & bn, Variable & v, const string & prefix )
{
	// Create a copy of the variable
	Variable * cl = v.clone();
	v.user_data = cl;
	cl->insert_to ( bn );
			
	// Append NTS name to annotation 
	AnnotString * as = find_origin ( cl->annotations );
	if ( !as )
		as = new AnnotString ( "origin", prefix + v.name );
	else
		as->value = prefix + as->value;

	cl->annotations.push_back ( as );
}

void transfer_to ( BasicNts & bn, State & s, const string & prefix )
{
	State * cl = new State ( s.name );
	s.user_data = cl;
	cl->insert_to ( bn );

	if ( s.is_error() )
		cl->is_error() = true;

	AnnotString * as = find_origin ( cl->annotations );
	if ( !as )
		as = new AnnotString ( "origin", prefix + s.name );
	else
		as->value = prefix + as->value;

	cl->annotations.push_back ( as );
}

class Inliner
{
	private:
		BasicNts & _bn;
		unsigned int _first_var_id;
		unordered_set < BasicNts * > _dests;

		void transfer_transition ( Transition & t );
		void inline_call_transition ( Transition & t, unsigned int id );
		void add_initial_final_states ( Transition & t);

	public:
		Inliner ( BasicNts & bn, unsigned int first_var_id ) :
			_bn ( bn ),
			_first_var_id ( first_var_id )
		{
			;
		}

		void find_destionation_ntses();
		void create_shadow_variables();
		unsigned int inline_call_transitions();
		void clear_user_pointers();		
		void normalize_names();
};

/**
 * @brief Transfers transition from called BasicNts to caller BasicNts.
 * @pre There must be new, unused control states in '_bn',
 *      end user pointers in callee should point to
 *      corresponding states in '_bn'.
 *      Also, all local variables, which are reffered to in 't',
 *      must have a copy in '_bn' (@ see substitute_variables ).
 */
void Inliner::transfer_transition ( Transition & t )
{
	TransitionRule * rule = t.rule().clone();
	if ( rule->kind() == TransitionRule::Kind::Call )
	{
		CallTransitionRule &r = *( CallTransitionRule * ) rule;
		for ( Term *t : r.terms_in() )
			substitute_variables ( *t );
		r.transform_return_variables ( substitute_const );
		
	} else {
		FormulaTransitionRule &r = *( FormulaTransitionRule * ) rule;
		substitute_variables ( r.formula() );
	}

	if ( !t.from().user_data || !t.to().user_data )
		throw logic_error ( "Unexpected nullptr" );

	State * from = (State * ) t.from().user_data;
	State * to   = (State * ) t.to().user_data;

	Transition * t2 = new Transition (
		unique_ptr < TransitionRule > ( rule ),
		*from,
		*to
	);

	t2->insert_to ( _bn );
}

/**
 * @pre: each state in destination must point to corresponding state in caller
 */
void Inliner::add_initial_final_states ( Transition & t )
{
	BasicNts & dest = ( ( CallTransitionRule & ) t.rule() ).dest();

	for ( State *s : dest.states() )
	{
		if ( s->is_initial() )
		{
			Transition * t_init = new Transition (
				std::make_unique < FormulaTransitionRule> (
					std::make_unique < Havoc > ()
				),
				t.from(),
				*static_cast < State * > ( s->user_data )
			);
			t_init->insert_to ( _bn );
		}

		if ( s->is_final() )
		{
			Transition * t_fin = new Transition (
				std::make_unique < FormulaTransitionRule > (
					std::make_unique < Havoc > ()
				),
				*static_cast < State * > ( s->user_data ),
				t.to()
			);
			t_fin->insert_to ( _bn );
		}
	}
}

/**
 * @pre All variables in destination BasicNts points to variables in _bn.
 */
void Inliner::inline_call_transition ( Transition & t, unsigned int id )
{
	/*
	 * 1 Add states + state mapping
	 * 2 Copy transitions and change
	 * 2.1 them to point between _bn states
	 * 2.2 their formulas to use _bn variables
	 * 3 Clear state mapping
	 */
	CallTransitionRule & ctr = (CallTransitionRule & ) t.rule();
	BasicNts & dest = ctr.dest();

	// Copy states
	string prefix = _bn.name + ":" + to_string ( id ) + ":";
	for ( State * s : dest.states() )
	{
		transfer_to ( _bn, *s, prefix );
	}

	// Transfer transitions
	for ( Transition *t : dest.transitions() )
	{
		transfer_transition ( *t );
	}

	add_initial_final_states ( t );

	// Clear user pointers
	for ( State *s : dest.states() )
	{
		s->user_data = nullptr;
	}
}


void Inliner::find_destionation_ntses()
{
	for ( Transition * t : _bn.transitions() )
	{
		if ( t->rule().kind() != TransitionRule::Kind::Call )
			continue;

		CallTransitionRule & ctr = ( CallTransitionRule &) t->rule();
		_dests.insert ( & ctr.dest() );
	}
}

void Inliner::create_shadow_variables()
{
	for ( BasicNts * b : _dests )
	{
		string prefix = b->name + "::";

		for ( Variable * v : b->variables() )
			transfer_to ( _bn, *v,  prefix );

		for ( Variable *v : b->params_in() )
			transfer_to ( _bn, *v, prefix );

		for ( Variable *v : b->params_out() )
			transfer_to ( _bn, *v, prefix );

	}
}

/**
 * @return number of inlined calls
 */
unsigned int Inliner::inline_call_transitions()
{
	unsigned int id = 0;
	for ( auto it = _bn.transitions().begin();
			it != _bn.transitions().end();
			)
	{
		Transition *t = *it;
		it++;

		if ( t->rule().kind() != TransitionRule::Kind::Call )
			continue;

		// Unlink && delete
		inline_call_transition ( *t, id );
		t->remove_from_parent();
		delete t;
		
		id++;
	}

	return id;
}

void Inliner::normalize_names()
{
	unsigned int st_id = 0;
	for ( State *s : _bn.states() )
	{
		s->name = string ( "st_" ) + to_string ( st_id );
		st_id++;
	}

	unsigned int var_id = _first_var_id;
	for ( Variable *v : _bn.variables() )
	{
		v->name = string ( "var_" ) + to_string ( var_id );
		var_id++;
	}

	for ( Variable * v : _bn.params_in() )
	{
		v->name = string ( "var_" ) + to_string ( var_id );
		var_id++;
	}

	for ( Variable * v : _bn.params_out() )
	{
		v->name = string ( "var_" ) + to_string ( var_id );
		var_id++;
	}
}

void Inliner::clear_user_pointers()
{

	for ( BasicNts * b : _dests )
	{
		for ( Variable * v : b->variables() )
		{
			v->user_data = nullptr;
		}
	}
}

/**
 * @pre All variables in destination BasicNtses must have 'origin' annotation
 * @return number of inlined calls
 */
unsigned int inline_calls ( BasicNts & bn, unsigned int first_var_id )
{
	Inliner iln ( bn, first_var_id );
	iln.find_destionation_ntses();
	iln.create_shadow_variables();
	unsigned int n = iln.inline_call_transitions();
	iln.normalize_names();
	iln.clear_user_pointers();

	return n;
}

/**
 * @pre There is no recursion, neither direct nor indirect.
 */
void inline_calls_simple ( Nts & nts )
{
	unordered_set < BasicNts * > root_ntses;

	// Annotate and rename global variables
	unsigned int var_id = 0;
	for ( Variable * v : nts.variables() )
	{
		auto * as = new AnnotString ( "origin", move ( v->name ) );
		v->annotations.push_back ( as );
		v->name = string ( "gvar_" ) + to_string ( var_id );
		var_id++;
	}

	for ( Instance * i : nts.instances() )
	{
		root_ntses.insert ( & i->basic_nts() );
	}

	auto root_ntses_copy = root_ntses;

	while ( !root_ntses.empty() )
	{
		for ( auto it = root_ntses.begin(); it != root_ntses.end(); )
		{
			BasicNts & bn = **it;
			auto curr = it;
			it++;

			unsigned int n = inline_calls ( bn, var_id );
			if ( n <= 0 )
				root_ntses.erase ( curr );
		}
	}

	// Remove ntses which are not in our set
	for ( auto it = nts.basic_ntses().begin(); it != nts.basic_ntses().end(); )
	{
		BasicNts * bn = *it;
		++it;

		const bool is_in = root_ntses_copy.find ( bn ) != root_ntses_copy.end();
		if ( !is_in )
		{
			bn->remove_from_parent();
			delete bn;
		}
	}
}

bool make_inline ( Nts & n )
{

	return false;
}
