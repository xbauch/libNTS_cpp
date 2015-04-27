#include <memory>
#include <stdexcept>
#include "sugar.hpp"

using std::unique_ptr;
using std::domain_error;
using std::move;

namespace nts {
namespace sugar
{
unique_ptr < Relation > operator== (
		unique_ptr < Term > & t1,
		unique_ptr < Term > & t2)
{
	return std::make_unique < Relation > (
			RelationOp::eq,
			move ( t1 ),
			move ( t2 )
	);
}

unique_ptr < Relation > operator< (
		unique_ptr < Term > & t1,
		unique_ptr < Term > & t2)
{
	return std::make_unique < Relation > (
			RelationOp::lt,
			move ( t1 ),
			move ( t2 )
	);
}

unique_ptr < Relation > operator<= (
		unique_ptr < Term > & t1,
		unique_ptr < Term > & t2)
{
	return std::make_unique < Relation > (
			RelationOp::leq,
			move ( t1 ),
			move ( t2 )
	);
}

unique_ptr < Relation > operator> (
		unique_ptr < Term > & t1,
		unique_ptr < Term > & t2)
{
	return std::make_unique < Relation > (
			RelationOp::gt,
			move ( t1 ),
			move ( t2 )
	);
}

unique_ptr < Relation > operator>= (
		unique_ptr < Term > & t1,
		unique_ptr < Term > & t2)
{
	return std::make_unique < Relation > (
			RelationOp::geq,
			move ( t1 ),
			move ( t2 )
	);
}

unique_ptr < Relation > operator!= (
		unique_ptr < Term > & t1,
		unique_ptr < Term > & t2)
{
	return std::make_unique < Relation > (
			RelationOp::neq,
			move ( t1 ),
			move ( t2 )
	);
}

unique_ptr < Relation > operator< (
		unique_ptr < Term > & t,
		int n )
{
	if ( n != 0 )
		throw domain_error ( "Only zero is supported" );

	if ( ! t->type().is_scalar() )
		throw domain_error ( "Only scalar types are supported" );

	const ScalarType & sc = t->type().scalar_type();
	if ( sc.is_bitvector() )
	{
		unsigned int w = sc.bitwidth();
		if ( w < 1 || w > 8 * sizeof(int) )
			throw domain_error ( "Bitwidth is too large" );

		unique_ptr < Term > n = std::make_unique < IntConstant > ( 1 << ( w - 1 ) );
		
		// If it is greater than this, it is negative
		return t >= n;
	}

	if ( ( sc == ScalarType::Integer() ) || ( sc == ScalarType::Real() ) )
	{
		unique_ptr < Term > n = std::make_unique < IntConstant > ( 0 );
		return t < n;
	}

	throw domain_error ( "Negativity test not supported on this type" );

}

nts::FormulaBop & operator== ( nts::Formula & f1, nts::Formula &f2 )
{
	return * new FormulaBop ( BoolOp::Equiv,
			unique_ptr<Formula> ( &f1 ),
			unique_ptr<Formula> ( &f2 ) );
}

FormulaBop & operator&& ( nts::Formula & f1, nts::Formula & f2 )
{
	return * new FormulaBop (
			BoolOp::And,
			unique_ptr < Formula > ( &f1 ),
			unique_ptr < Formula > ( &f2 )
	);
}

unique_ptr < FormulaBop > operator&& (
		unique_ptr < Formula > && f1,
		unique_ptr < Formula > && f2  )
{
	return unique_ptr < FormulaBop > ( & (*f1.release() && *f2.release() ) );
}


unique_ptr < FormulaBop > operator== (
		unique_ptr < Formula > && f1,
		unique_ptr < Formula > && f2 )
{
	return unique_ptr < FormulaBop > ( & ( *f1.release() == *f2.release() ) );
}

unique_ptr < FormulaBop > operator== (
		unique_ptr < Formula > & f1,
		unique_ptr < Formula > & f2 )
{
	return move ( f1 ) == move ( f2 );
}


unique_ptr < FormulaBop > equally_negative (
		unique_ptr < Term > & t1,
		unique_ptr < Term > & t2 )
{
	unique_ptr < Formula > ap1 = t1 < 0;
	unique_ptr < Formula > ap2 = t2 < 0;
	return ap1 == ap2;
}

Relation & operator== ( Term & t1, Term & t2 )
{
	return * new Relation (
			RelationOp::eq,
			unique_ptr < Term > ( &t1 ),
			unique_ptr < Term > ( &t2 )
	);
}

Relation & operator== ( Term & t1, int t2 )
{
	return ( t1 == *( new IntConstant ( t2 ) ) );
}

Relation & operator> ( Term & t1, Term & t2 )
{
	return * new Relation (
			RelationOp::gt,
			unique_ptr < Term > ( & t1 ),
		   	unique_ptr < Term > ( & t2 )
	);
}

nts::Relation & operator> ( nts::Term & t1, int t2 )
{
	return (  t1  > *( new IntConstant ( t2 )  ) );
}

Relation & operator>= ( Term & t1, Term & t2 )
{
	return * new Relation (
			RelationOp::geq,
			unique_ptr < Term > ( & t1 ),
		   	unique_ptr < Term > ( & t2 )
	);
}

nts::Relation & operator>= ( nts::Term & t1, int t2 )
{
	return (  t1  >= *( new IntConstant ( t2 )  ) );
}

Relation & operator< ( Term & t1, Term & t2 )
{
	return * new Relation (
			RelationOp::lt,
			unique_ptr < Term > ( & t1 ),
		   	unique_ptr < Term > ( & t2 )
	);
}

nts::Relation & operator< ( nts::Term & t1, int t2 )
{
	return ( t1 < *( new IntConstant ( t2 )  ) );
}

Relation & operator<= ( Term & t1, Term & t2 )
{
	return * new Relation (
			RelationOp::leq,
			unique_ptr < Term > ( & t1 ),
		   	unique_ptr < Term > ( & t2 )
	);
}

nts::Relation & operator<= ( nts::Term & t1, int t2 )
{
	return ( t1 <= *( new IntConstant ( t2 )  ) );
}

ArithmeticOperation & operator+ ( Term & t1, Term & t2 )
{
	return * new ArithmeticOperation (
			ArithOp::Add,
			unique_ptr < Term > ( & t1 ),
			unique_ptr < Term > ( & t2 )
	);
}

ArithmeticOperation & operator+ ( Term & t1, int t2 )
{
	return ( t1 + *( new IntConstant ( t2 ) ) );
}

ThreadID & tid()
{
	return * new ThreadID();
}

Havoc & havoc ()
{
	return * new Havoc ();
}

Havoc & havoc ( std::initializer_list < Variable *> vars )
{
	return * new Havoc ( vars );
}

VariableReference & CURR ( const Variable & var )
{
	return * new VariableReference ( var, false );
}

VariableReference & CURR ( const Variable * var )
{
	return CURR ( *var );
}


VariableReference & NEXT ( const Variable & var )
{
	return * new VariableReference ( var, true );
}

VariableReference & NEXT ( const Variable * var )
{
	return NEXT ( *var );
}

SugarTransitionStates::SugarTransitionStates ( State & from, State & to ) :
	_from ( from ),
	_to   ( to   )
{
	;
}

Transition & SugarTransitionStates::operator () ( Formula & f )
{
	return * new Transition (
		std::make_unique < FormulaTransitionRule > (
			unique_ptr < Formula > ( &f )
		),
		_from,
		_to
	);
}

Transition & SugarTransitionStates::operator () ( unique_ptr < Formula > f )
{
	return * new Transition (
		std::make_unique < FormulaTransitionRule > (
			move ( f )
		),
		_from,
		_to
	);
}

SugarTransitionStates operator ->* ( State & from, State & to )
{
	return SugarTransitionStates ( from, to );
}

ArrRead::ArrRead ( nts::Variable & arr_var ) :
	_arr_var ( arr_var )
{
	;
}

ArrayTerm & ArrRead::operator [] ( Term & t )
{
	return * new ArrayTerm (
			std::make_unique < VariableReference > ( _arr_var, false ),
			{ & t }
	);
}

ArrWriting::ArrWriting ( const Variable & arr_var, Term & t ) :
	_arr_var ( arr_var ),
	_idx     ( & t     )
{
	;
}

ArrayWrite & ArrWriting::operator== ( Term & value )
{
	if ( ! _idx )
		throw std::logic_error ( "ArrWriting::operator== used twice!" );

	Term * idx = _idx;
	_idx = nullptr;

	return * new nts::ArrayWrite ( _arr_var, {},  { idx }, { & value } );
}

ArrayWrite & ArrWriting::operator== ( int value )
{
	return operator== ( * new IntConstant ( value ) );
}

ArrWrite::ArrWrite ( const Variable & arr_var ) :
	_arr_var ( arr_var )
{
	;
}

ArrWriting ArrWrite::operator[] ( Term & idx )
{
	return ArrWriting ( _arr_var, idx );
}



} // namespace sugar
} // namespace nts
