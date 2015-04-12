#include <algorithm>
#include <utility>

#include "nts.hpp"
#include "logic.hpp"

using namespace nts;
using std::unique_ptr;
using std::move;

//------------------------------------//
// Term                               //
//------------------------------------//

Term::Term ( bool minus, DataType t ) :
	_minus ( minus ),
	_type  ( t     )
{
	;
}

Term::Term ( const Term & orig ) :
	_minus ( orig._minus ),
	_type  ( orig._type  )
{
	;
}

Term * Term::clone() const
{
	return new Term ( *this );
}

//------------------------------------//
// FormulaBop                         //
//------------------------------------//

FormulaBop::FormulaBop ( BoolOp op,
		unique_ptr<Formula> f1,
		unique_ptr<Formula> f2 ) :
	_op ( op ),
	_f { move ( f1 ), move ( f2 ) }
{
	;
}

FormulaBop::FormulaBop ( const FormulaBop & orig ) :
	_op ( orig._op )
{
	_f[0] = unique_ptr<Formula> ( orig._f[0]->clone() );
	_f[1] = unique_ptr<Formula> ( orig._f[1]->clone() );
}

FormulaBop::FormulaBop ( FormulaBop && old ) :
	_op ( old._op )
{
	_f[0] = move ( old._f[0] );
	_f[1] = move ( old._f[1] );
}

const Formula & FormulaBop::formula_1() const
{
	return *_f[0];
}

const Formula & FormulaBop::formula_2() const
{
	return *_f[1];
}

FormulaBop * FormulaBop::clone() const
{
	return new FormulaBop ( *this );
}

//------------------------------------//
// FormulaNot                         //
//------------------------------------//

FormulaNot::FormulaNot ( unique_ptr<Formula> f ) :
	_f ( move(f) )
{
	;
}

FormulaNot::FormulaNot ( const FormulaNot & orig )
{
	_f = unique_ptr<Formula> ( orig._f->clone() );
}

FormulaNot::FormulaNot ( FormulaNot && old )
{
	_f = move ( old._f );
}

const Formula & FormulaNot::formula() const
{
	return *_f;
}

FormulaNot * FormulaNot::clone() const
{
	return new FormulaNot ( *this );
}

//------------------------------------//
// QuantifiedType                     //
//------------------------------------//

QuantifiedType::QuantifiedType ( DataType t ) :
	_t    ( t ),
	_from ( nullptr ),
	_to   ( nullptr )
{
	;
}

QuantifiedType::QuantifiedType ( DataType t,
		unique_ptr<Term> from,
		unique_ptr<Term> to  ) :
	_t    ( t                  ),
	_from ( move ( from ) ),
	_to   ( move ( to   ) )
{
	if ( from->type() != t || to->type() != t )
		throw TypeError();
}

QuantifiedType::QuantifiedType ( const QuantifiedType & orig ) :
	_t    ( orig._t ),
	_from ( nullptr ),
	_to   ( nullptr )
{
	if ( orig._from )
	{
		_from = unique_ptr<Term> ( orig._from->clone() );
		_to   = unique_ptr<Term> ( orig._to->clone() );
	}
}

QuantifiedType::QuantifiedType ( QuantifiedType && old ) :
	_t ( move ( old._t ) )
{
	_from = move ( old._from );
	_to   = move ( old._to   );
}

//------------------------------------//
// QuantifiedVariableList             //
//------------------------------------//

QuantifiedVariableList::QuantifiedVariableList
(
		Quantifier             q,
		const QuantifiedType & type
):
	_q     ( q    ),
	_qtype ( type )

{
	;
}

QuantifiedVariableList::QuantifiedVariableList
(
		Quantifier              q,
		const QuantifiedType && type
):
	_q     ( q    ),
	_qtype ( type )

{
	;
}

QuantifiedVariableList::QuantifiedVariableList ( const QuantifiedVariableList & orig ) :
	_q     ( orig._q     ),
	_qtype ( orig._qtype )
{
	for ( auto * v : orig._vars )
	{
		auto * clone = v->clone();
		_vars.push_back ( clone );
	}
}

QuantifiedVariableList::QuantifiedVariableList ( QuantifiedVariableList && old ) :
	_q     ( move ( old._q     ) ),
	_qtype ( move ( old._qtype ) )
{
	_vars = move ( old._vars );
	old._vars.clear();

	for ( auto *v : _vars )
	{
		v->remove_from_parent();
		v->insert_to ( *this );
	}
}

QuantifiedVariableList::~QuantifiedVariableList ()
{
	for ( auto *v : _vars )
	{
		delete v;
	}
}


//------------------------------------//
// QuantifiedFormula                  //
//------------------------------------//

QuantifiedFormula::QuantifiedFormula (
				Quantifier              q,
				const QuantifiedType &  type,
				unique_ptr<Formula>     f    ) :
	_qvlist ( q, type         ),
	_f      ( move ( f ) )
{
	;
}

QuantifiedFormula::QuantifiedFormula (
				Quantifier              q,
				const QuantifiedType && type,
				unique_ptr<Formula>     f    ) :
	_qvlist ( q, type         ),
	_f      ( move ( f ) )
{
	;
}

QuantifiedFormula::QuantifiedFormula ( const QuantifiedFormula & orig ) :
	_qvlist ( orig._qvlist )
{
	_f = unique_ptr<Formula> ( orig._f->clone() );
}

QuantifiedFormula::QuantifiedFormula ( QuantifiedFormula && old ) :
	_qvlist ( move ( old._qvlist ) ),
	_f      ( move ( old._f      ) )
{
	;
}

QuantifiedFormula * QuantifiedFormula::clone() const
{
	return new QuantifiedFormula ( *this );
}


//------------------------------------//
// Havoc                              //
//------------------------------------//

Havoc::Havoc ( const std::initializer_list < const Variable *> & vars ) :
	_vars ( vars )
{
	;
}

Havoc::Havoc ( const Havoc & orig ) :
	_vars ( orig._vars )
{
	;
}

Havoc::Havoc ( Havoc && old ) :
	_vars ( move ( old._vars ) )
{
	;
}

Havoc * Havoc::clone() const
{
	return new Havoc ( *this );
}

//------------------------------------//
// BooleanTerm                        //
//------------------------------------//

BooleanTerm::BooleanTerm ( unique_ptr<Term> t )
{
	if ( t->type() != DataType::Bool() )
		throw TypeError();

	_t = move(t);
}

BooleanTerm::BooleanTerm ( const BooleanTerm & orig )
{
	_t = unique_ptr<Term> ( orig._t->clone() );
}

BooleanTerm::BooleanTerm ( BooleanTerm && old )
{
	_t = move ( old._t );
}

BooleanTerm * BooleanTerm::clone() const
{
	return new BooleanTerm ( *this );
}

//------------------------------------//
// Relation                           //
//------------------------------------//

void Relation::check_type ( const DataType &t1, const DataType &t2 )
{
	if ( t1.is_bitvector() && t2.is_bitvector() )
		return;

	if ( t1 != t2 )
		throw TypeError();
}

Relation::Relation ( RelationOp op, unique_ptr<Term> t1, unique_ptr<Term> t2 ) :
	_op ( op          ),
	_t1 ( move ( t1 ) ),
	_t2 ( move ( t2 ) )
{
	check_type( t1->type(), t2->type() );
}

Relation::Relation ( const Relation & orig ) :
	_op ( orig._op )
{
	_t1 = unique_ptr<Term> ( orig._t1->clone() );
	_t2 = unique_ptr<Term> ( orig._t2->clone() );
}

Relation::Relation ( Relation && old ) :
	_op ( move ( old._op ) ),
	_t1 ( move ( old._t1 ) ),
	_t2 ( move ( old._t2 ) )
{
	;
}

Relation * Relation::clone() const
{
	return new Relation ( *this );
}

//------------------------------------//
// ArithmeticOperation                //
//------------------------------------//

ArithmeticOperation::ArithmeticOperation ( ArithOp op,
				unique_ptr < Term > t1,
				unique_ptr < Term > t2 ) :
	Term ( false, calc_type ( t1.get(), t2.get() ) ),
	_op ( op ),
	_t1 ( move ( t1 ) ),
	_t2 ( move ( t2 ) )
{
	;
}

DataType ArithmeticOperation::calc_type ( const Term * term1, const Term *term2 )
{
	const DataType &t1 = term1->type();
	const DataType &t2 = term2->type();

	// Same types
	if ( t1 == t2 )
		return t1;

	// t1 can be whatever type of class Integral (probably constant)
	// t2 is some concrete type of class Integral,
	// or whatever type of class Integral
	if ( t1 == DataType::Integral() && t2.is_integral() )
		return t2;

	// Commutatively
	if ( t2 == DataType::Integral() && t1.is_integral() )
		return t1;

	// Both are BitVectors, but have different size (because t1 != t2)
	if ( t1.is_bitvector() && t2.is_bitvector() )
	{
		return DataType::BitVector ( std::max( t1.bitwidth(), t2.bitwidth() ) );
	}

	throw TypeError();
}

const ArithOp & ArithmeticOperation::operation() const
{
	return _op;
}

const Term & ArithmeticOperation::term1() const
{
	return *_t1;
}

const Term & ArithmeticOperation::term2() const
{
	return *_t2;
}

ArithmeticOperation * ArithmeticOperation::clone() const
{
	return new ArithmeticOperation ( _op,
			unique_ptr<Term>(_t1->clone()),
			unique_ptr<Term>(_t2->clone())
			);
}

//------------------------------------//
// IntConstant                        //
//------------------------------------//

IntConstant::IntConstant ( int value ) :
	Constant ( DataType::Integral() ),
	_value   ( value )
{
	;
}

IntConstant * IntConstant::clone() const
{
	return new IntConstant ( _value );
}

//------------------------------------//
// VariableReference n                //
//------------------------------------//

VariableReference::VariableReference ( const Variable &var, bool primed ) :
	Leaf    ( var.type() ),
	_var    ( &var       ),
	_primed ( primed     )
{
	;
}

VariableReference * VariableReference::clone() const
{
	return new VariableReference ( *_var, _primed );
}

