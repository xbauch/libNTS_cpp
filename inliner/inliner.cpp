#include <map>
#include <string>
#include <utility>
#include <unordered_set>
#include <algorithm>
#include <stdexcept>

#include <libNTS/nts.hpp>
#include <libNTS/logic.hpp>

using namespace nts;

using std::map;
using std::string;
using std::unordered_set;
using std::make_pair;
using std::find_if;
using std::logic_error;
using std::unique_ptr;

/*
 * Jak to udelat s nazvy promennych?
 * Pridame jednoduche scope resolution pomoci ctyrtecky ( :: ).
 * To neni legalni znak, a tak pred exportem vysledku nahradime vsechny vyskyty
 * tohoto stringu necim neskodnym (napr. stringem "II"). No jo, ale to neco neskodneho se nesmi vyskytovat
 * ve funkcich predtim! Tedy tu vec hned na zacatku nahradime napriklad "I_I",
 * tedy kazdy po sobe jdouci druhy vyskyt "I" nahradime "_I".
 * OO __ => _i_Y ahojYYsvete
 *
 * Ale co kdyz existuji promenne "promI_I" a "promII"?
 *
 * Pamatujeme si vsechny promenne se stejnym prefixem (vyraz v prvnich zavorkach)
 * ([^0-9I]*)[0-9]*I.*
 * Muzeme si je take drzet serazene.
 *
 * A co kdybych se vykaslal na zachovavani nazvu promennych,
 * vsechny je pojmenovaval nejak nesmyslene ( "prom_17" )
 * a u kazde promenne si pamatoval v anotaci, odkud prisla?
 *
 * Ukazka promenne, ktera puvodne byla promennou v basic_3,
 * nez byl basic_3 inlinovay z basic_1.
 * @orig:string:"basic_1::basic_3::i";
 *
 * Na zacatku kazdou promennou oanotuji "i" - jeji puvodni nazev.
 * Pokud se BasicNTS, ve kterem se promenna nachazi, nainlinuje do jineho,
 * prida se nazev volaneho BasicNts do anotace jako prefix.
 *
 * Pokud nekdo dokaze potom z anotaci pojmenovat dane promenne tak,
 * aby to davalo smysl - necht to udela.
 *
 * V anotacich muzu pouzivat ty krasne ctyrtecky, ale v nazvech promennych ne.
 *
 *
 * Btw to same muzu pozdeji udelat pro stavy.
 */

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
		_visible_names.insert ( make_pair ( v->name(), v ) );

	for ( Variable *v : _bn.params_out() )
		_visible_names.insert ( make_pair ( v->name(), v ) );

	for ( Variable *v : _bn.variables() )
		_visible_names.insert ( make_pair ( v->name(), v ) );

	if ( _bn.parent() )
		for ( Variable *v : _bn.parent()->variables() )
			_visible_names.insert ( make_pair ( v->name(), v ) );
}


void annotate_variable ( Variable & v )
{
	Annotation * a = new AnnotString ( "origin", v.name() );
	a->insert_to ( v.annotations );
}

/*
 * T must provide member access to 'annotations ( list of annotations)'
 * and to 'std::string name() const'
 */
template < typename T >
void annotate_with_origin ( T & x )
{
	Annotation * a = new AnnotString ( "origin", x.name() );
	a->insert_to ( x.annotations );
}


/*
 * @post Every variables has an "origin" annotation
 */
void annotate_variables ( Nts & n )
{
	// Global variables
	for ( Variable *v : n.variables() )
		annotate_variable ( *v );

	for ( BasicNts * bn : n.basic_ntses() )
	{
		for ( Variable *v : bn->params_in() )
			annotate_variable ( *v );

		for ( Variable *v : bn->params_out() )
			annotate_variable ( *v );

		for ( Variable *v : bn->variables() )
			annotate_variable ( *v );
	}
}

void annotate_states ( Nts & n )
{
	for ( BasicNts * bn : n.basic_ntses() )
	{
		for ( State *s : bn->states() )
			annotate_with_origin ( *s );
	}
}

// Chtel bych si pro kazdy BasicNts drzet
// seznam (s konstantnim pristupem - pole )
// promennych, ocislovanych indexy pole.
// Prvky toho pole by ukazovaly jednak na danou promennou ve volanem
// BasicNts, tak i na odpovidajici (nove vytvorenou) promennou
//
// Dale, kazda promenna by si drzela seznam formuli, ktere na ni odkazuji
//
//
// Hlavni otazka: Jak efektivne zduplikovat formuli tak, aby ve vysledku pouzivala
// jine promenne?
// Odpoved: Zduplikujeme ji normalne. TODO


/**
 *@brief Replace transition with given call rule with 
 */
void inline_call ( CallTransitionRule & r )
{

}

AnnotString * find_origin ( Annotations & ants )
{
	Annotations::iterator it = find_if ( ants.begin(), ants.end(), 
			[] ( const Annotation *a) -> bool
			{
				return a->name() == "origin" && a->type() == Annotation::Type::String;
			}
	);

	if ( it == ants.end() )
		return nullptr;

	return static_cast < AnnotString * > ( *it );
}

void substitute_variables ( AtomicProposition & ap );
void substitute_variables ( Term & t );
void substitute_variables ( Formula & f );

unique_ptr < Term > substitute_term ( unique_ptr < Term > t )
{
	substitute_variables ( *t );
	return t;
}



Variable * substitute ( Variable * var )
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
 * or poinring to another variable.
 * @post Occurences of variables, which had 'user data' pointing to some other variable,
 * are substituted with the other variable.
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
void transfer_to ( BasicNts & bn, Variable & v, const string prefix )
{
	// Create a copy of the variable
	Variable * cl = v.clone();
	v.user_data = cl;
	cl->insert_to ( bn );
			
	// Append NTS name to annotation 
	AnnotString * as = find_origin ( cl->annotations );
	if ( !as )
		throw logic_error ( "Precondition failed: missing 'origin' annotation" );

	string new_origin = prefix + as->value();
	as->value() = move ( new_origin );		
}

/**
 * @pre All variables in destination BasicNtses must have 'origin' annotation
 */
void inline_calls ( BasicNts & bn )
{
	// Find all destination NTSes
	unordered_set < BasicNts * > dests;
	for ( auto & i : bn.callees() )
	{
		dests.insert ( & i.dest() );
	}

	// Create shadow variables
	for ( BasicNts * b : dests )
	{
		string prefix = b->name() + "::";

		for ( Variable * v : b->variables() )
			transfer_to ( bn, *v,  prefix );

		for ( Variable *v : b->params_in() )
			transfer_to ( bn, *v, prefix );

		for ( Variable *v : b->params_out() )
			transfer_to ( bn, *v, prefix );

	}


	// Clear all used user pointers
	
	for ( BasicNts * b : dests )
	{
		for ( Variable * v : b->variables() )
		{
			v->user_data = nullptr;
		}
	}
}

bool make_inline ( Nts & n )
{

	return false;
}
