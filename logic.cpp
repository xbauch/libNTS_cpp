#include <algorithm>

#include "nts.hpp"
#include "logic.hpp"

using namespace nts;

//------------------------------------//
// Term                               //
//------------------------------------//

Term::Term ( bool minus, DataType t ) :
	_minus ( minus ),
	_type  ( t     )
{
	;
}

const Term * Relation::term1() const
{
	return _t1;
}

const Term * Relation::term2() const
{
	return _t2;
}

//------------------------------------//
// BooleanTerm                        //
//------------------------------------//


BooleanTerm::BooleanTerm ( const Term * t ) :
	_t ( t )
{
	if ( t->type() != DataType::Bool() )
		throw TypeError();
}

//------------------------------------//
// FormulaBop                         //
//------------------------------------//

FormulaBop::FormulaBop ( BoolOp op, const Formula *f1, const Formula *f2 ) :
	_op ( op ),
	_f {f1, f2 }
{
	;
}

const Formula * FormulaBop::formula_1() const
{
	return _f[0];
}

const Formula * FormulaBop::formula_2() const
{
	return _f[1];
}

//------------------------------------//
// FormulaBop                         //
//------------------------------------//

FormulaNot::FormulaNot ( const Formula *f ) :
	_f ( f )
{
	;
}

const Formula * FormulaNot::formula() const
{
	return _f;
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

QuantifiedType::QuantifiedType ( DataType t, const Term * from, const Term * to  ) :
	_t    ( t    ),
	_from ( from ),
	_to   ( to   )
{
	if ( from->type() != t || to->type() != t )
		throw TypeError();
}

//------------------------------------//
// QuantifiedFormula                  //
//------------------------------------//


QuantifiedFormula::QuantifiedFormula ( 
				Quantifier q,
				std::initializer_list < const Variable * > variables,
				const QuantifiedType & type,
				const Formula * f ) :
	_q     ( q    ),
	_qtype ( type ),
	_f     ( f    )
{
	for ( const Variable * v : variables )
	{
		if ( v->type() != type.type() )
			throw TypeError();
	}

	_vars.insert ( _vars.end(), variables );
}


//------------------------------------//
// Havoc                              //
//------------------------------------//

Havoc::Havoc ( const std::initializer_list < const Variable *> & vars ) :
	_vars ( vars )
{
	;
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

Relation::Relation ( RelationOp op, const Term * t1, const Term * t2 ) :
	_op ( op ),
	_t1 ( t1 ),
	_t2 ( t2 )
{
	check_type( t1->type(), t2->type() );
}

const RelationOp & Relation::operation() const
{
	return _op;
}

//------------------------------------//
// ArithmeticOperation                //
//------------------------------------//

ArithmeticOperation::ArithmeticOperation ( ArithOp op, const Term *t1, const Term *t2 ) :
	Term ( false, calc_type ( t2, t2 ) ),
	_op ( op ),
	_t1 ( t1 ),
	_t2 ( t2 )
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

const Term * ArithmeticOperation::term1() const
{
	return _t1;
}

const Term * ArithmeticOperation::term2() const
{
	return _t2;
}

