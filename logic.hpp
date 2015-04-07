#ifndef NTS_LOGIC_HPP_
#define NTS_LOGIC_HPP_
#pragma once

#include <vector>
#include <initializer_list>
#include "data_types.hpp"


namespace nts
{

// Forward declaration (not to depend on nts.hpp)
class Variable;

enum class BoolOp
{
	And,
	Or,
	Imply,
	Equiv
};

enum class ArithOp
{
	Add,
	Sub,
	Mul,
	Div,
	Mod
};

enum class RelationOp
{
	eq,  // =  Equal
	neq, // != Not equal
	leq, // <= Less than or equal to
	lt,  // <  Less than
	geq, // >= Greather than or equal to
	gt   // >  Greater than
};

enum class Quantifier
{
	Forall,
	Exists
};

class Term
{
	private:
		bool     _minus;
		DataType _type;

	public:
		// type can be whatever type
		Term ( bool minus, DataType type ); 

		const DataType & type () const { return _type; }
};

// Formulas have always type Bool
// It can be:
// * Atomic proposition
// * FormulaNot
// * FormulaBop
class Formula
{
	public:
		Formula()          = default;
		virtual ~Formula() = default;
};

class FormulaBop : public Formula
{
	private:
		BoolOp _op;
		const Formula * _f[2];

	public:
		FormulaBop ( BoolOp op, const Formula *f1, const Formula *f2 );
		const Formula * formula_1 () const;
		const Formula * formula_2 () const;
};

class FormulaNot : public Formula
{
	private:
		const Formula * _f;

	public:
		FormulaNot ( const Formula * f );

		const Formula * formula() const;
};

class QuantifiedType
{
	private:
		DataType     _t;
		const Term * _from;
		const Term * _to;

	public:
		QuantifiedType ( DataType t );
		QuantifiedType ( DataType t, const Term * from, const Term * to );

		const DataType & type () const { return _t; }
		const Term * from() { return _from; }
		const Term * to()   { return _to;   }
};

class QuantifiedFormula : public Formula
{
	private:
		Quantifier _q;
		std::vector < const Variable * > _vars;
		QuantifiedType _qtype;
		const Formula * _f;

	public:
		QuantifiedFormula (
				Quantifier q,
				std::initializer_list < const Variable * > variables,
				const QuantifiedType & type,
				const Formula * f );

		const std::vector < const Variable * > & variables() const
		{ return _vars; }
		const QuantifiedType & qtype() const
		{ return _qtype; }
		const Formula * formula() const
		{ return _f; }
		const Quantifier & quantifier() const
		{ return _q; }
};

// Atomic proposition is a:
// * BooleanTerm
// * Havoc
// * Relation
// * ArrayWrite ( not implemented )
class AtomicProposition : public Formula
{

};

class Havoc : public AtomicProposition
{
	private:
		std::vector < const Variable *> _vars;

	public:
		Havoc ( const std::initializer_list < const Variable * > & list );
		const std::vector < const Variable *> & variables () const;
};

class BooleanTerm : public AtomicProposition
{
	private:
		const Term * _t;

	public:
		BooleanTerm ( const Term * t);

		const Term * term () const;
};

class Relation : public AtomicProposition
{
	private:
		RelationOp   _op;
		const Term * _t1;
		const Term * _t2;

		static void check_type ( const DataType &t1, const DataType &t2 );

	public:
		Relation ( RelationOp op, const Term *t1, const Term *t2 );

		const RelationOp & operation() const;
		const Term * term1() const;
		const Term * term2() const;
};


class ArithmeticOperation : public Term
{
	private:
		ArithOp      _op;
		const Term * _t1;
		const Term * _t2;

		static DataType calc_type ( const Term *t1, const Term *t2 );

	public:

		ArithmeticOperation ( ArithOp op, const Term *t1, const Term *t2 );

		const ArithOp & operation() const;
		const Term * term1() const;
		const Term * term2() const;
};

class Leaf : Term
{
	public:
		Leaf ( DataType type ) :
			Term ( false, type )
		{ ; }
};

class Constant : public Leaf
{
	public:
		Constant ( DataType type ) :
			Leaf ( type )
		{ ; }
};

class IntConstant : public Constant
{
	private:
		int _value;

	public:
		IntConstant ( int value );
};


class VariableReference : public Leaf
{
	private:
		const Variable * _var;
		bool             _primed;

	public:
		VariableReference ( const Variable *var, bool primed );
};

#if 0
class Formula
{
	public:
		Formula();

		class Variables;
		class PrimedVariables;

		Variables variables() const;
		PrimedVariables primed_variables() const;
};

class Formula::Variables
{
	public:
		class iterator;

		iterator begin();
		iterator end();
};

class Formula::PrimedVariables
{
	public:
		class iterator;

		iterator begin();
		iterator end();
};
#endif

}

#endif // NTS_LOGIC_HPP_
