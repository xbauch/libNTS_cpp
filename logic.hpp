#ifndef NTS_LOGIC_HPP_
#define NTS_LOGIC_HPP_
#pragma once

#include <ostream>
#include <vector>
#include <list>
#include <initializer_list>
#include <memory>
#include "data_types.hpp"

// TODO: Substitute term with another term

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

	protected:
		virtual void print ( std::ostream & o ) const = 0;

	public:
		// type can be whatever type
		Term ( bool minus, DataType type ); 
		Term ( const Term & orig );
		virtual ~Term() = default;

		const DataType & type () const { return _type; }

		// Caller is responsible to manage this
		// (can not use unique_ptr, they are not covariant )
		// Result is guaranteed to be non-null
		// Every child should override this function
		// and return its own type.
		virtual Term * clone() const = 0;

		friend std::ostream & operator<< ( std::ostream & o, const Term & t );
};

// Formulas have always type Bool
// It can be:
// * Atomic proposition
// * FormulaNot
// * FormulaBop
class Formula
{
	protected:
		virtual void print ( std::ostream & o ) const = 0;

	public:
		Formula()          = default;
		virtual ~Formula() = default;

		virtual Formula * clone() const = 0;

		friend std::ostream & operator<< ( std::ostream &, const Formula & );
};

// TODO: Some syntactic sugar for those formulas?

class FormulaBop : public Formula
{
	private:
		BoolOp _op;
		std::unique_ptr<Formula> _f[2];

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		FormulaBop ( BoolOp op,
				std::unique_ptr<Formula> f1,
				std::unique_ptr<Formula> f2 );
		
		FormulaBop ( const FormulaBop & orig );
		FormulaBop ( FormulaBop && old );
		virtual ~FormulaBop() = default;

		const Formula & formula_1 () const;
		const Formula & formula_2 () const;

		virtual FormulaBop * clone() const override;
};

class FormulaNot : public Formula
{
	private:
		std::unique_ptr<Formula>  _f;
		
	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		explicit FormulaNot ( std::unique_ptr<Formula> f );
		FormulaNot ( const FormulaNot & orig );
		FormulaNot ( FormulaNot && old );
		virtual ~FormulaNot() = default;

		const Formula & formula() const;

		virtual FormulaNot * clone() const override;
};

class QuantifiedType
{
	private:
		DataType     _t;

		std::unique_ptr<Term> _from;
		std::unique_ptr<Term> _to;
		
	public:
		QuantifiedType ( DataType t );
		QuantifiedType ( DataType && t );
		QuantifiedType ( DataType t,
				std::unique_ptr<Term> from,
				std::unique_ptr<Term> to );

		QuantifiedType ( const QuantifiedType & orig );
		QuantifiedType ( QuantifiedType && old );

		const DataType & type () const { return _t; }
		// may be null
		const Term * from() { return _from.get(); }
		const Term * to()   { return _to.get();   }

		friend std::ostream & operator<< ( std::ostream & o, const QuantifiedType & qt );
};

// Owns all variables inserted in Variable::insert_to()
class QuantifiedVariableList
{
	private:
		Quantifier               _q;
		QuantifiedType           _qtype;
		std::list < Variable * > _vars;

		friend class Variable;

	public:
		QuantifiedVariableList ( Quantifier q, const QuantifiedType &  qtype );
		QuantifiedVariableList ( Quantifier q, const QuantifiedType && qtype );
		QuantifiedVariableList ( const QuantifiedVariableList & orig );
		QuantifiedVariableList ( QuantifiedVariableList && old );

		~QuantifiedVariableList();

		Quantifier                  & quantifier() { return _q; }
		const Quantifier            & quantifier() const { return _q; }
		const QuantifiedType        & qtype()      const { return _qtype; }
		const std::list<Variable *> & variables()  const { return _vars; }

		friend std::ostream & operator<< ( std::ostream & o,
				const QuantifiedVariableList & qvl );
};

class QuantifiedFormula : public Formula
{
	private:
		QuantifiedVariableList   _qvlist;
		std::unique_ptr<Formula> _f;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		QuantifiedFormula (
				Quantifier                 q,
				const QuantifiedType     & type,
				std::unique_ptr<Formula>   f    );

		QuantifiedFormula (
				Quantifier                   q,
				const QuantifiedType     &&  type,
				std::unique_ptr<Formula>     f    );

		QuantifiedFormula ( const QuantifiedFormula & orig );
		QuantifiedFormula ( QuantifiedFormula && old );

		virtual ~QuantifiedFormula() = default;

		const QuantifiedVariableList & list() const { return _qvlist; }

		virtual QuantifiedFormula * clone() const override;
};

// Atomic proposition is a:
// * BooleanTerm
// * Havoc
// * Relation
// * ArrayWrite ( not implemented ) : TODO
class AtomicProposition : public Formula
{
	virtual AtomicProposition * clone() const override = 0;
};

class Havoc : public AtomicProposition
{
	private:
		std::vector < const Variable *> _vars;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		explicit Havoc ( const std::initializer_list < const Variable * > & list );
		Havoc ( const Havoc & orig );
		Havoc ( Havoc && old );
		virtual ~Havoc() = default;

		const std::vector < const Variable *> & variables () const;

		virtual Havoc * clone() const override;
};

class BooleanTerm : public AtomicProposition
{
	private:
		std::unique_ptr<Term> _t;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		explicit BooleanTerm ( std::unique_ptr<Term> t);
		BooleanTerm ( const BooleanTerm & orig );
		BooleanTerm ( BooleanTerm && old );
		virtual ~BooleanTerm() = default;

		const Term & term () const { return *_t; }

		virtual BooleanTerm * clone() const override;
};

class Relation : public AtomicProposition
{
	private:
		RelationOp            _op;
		std::unique_ptr<Term> _t1;
		std::unique_ptr<Term> _t2;
		// Both _t1 and _t2 are coerced to _type
		DataType              _type;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		Relation ( RelationOp op,
				std::unique_ptr<Term> t1,
				std::unique_ptr<Term> t2 );

		Relation ( const Relation & orig );
		Relation ( Relation && old );
		virtual ~Relation() = default;

		const RelationOp & operation() const { return _op; }
		const Term & term1() const { return *_t1; }
		const Term & term2() const { return *_t2; }

		const DataType & type() const { return _type; }

		virtual Relation * clone() const override;
};


class ArithmeticOperation : public Term
{
	private:
		using p_Term = std::unique_ptr < Term >;

		ArithOp _op;
		p_Term  _t1;
		p_Term  _t2;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:

		ArithmeticOperation ( ArithOp op,
				std::unique_ptr < Term > t1,
				std::unique_ptr < Term > t2 );

		ArithmeticOperation ( const ArithmeticOperation & orig );
		ArithmeticOperation ( ArithmeticOperation && old );
		virtual ~ArithmeticOperation() = default;

		const ArithOp & operation() const;
		const Term & term1() const;
		const Term & term2() const;

		virtual ArithmeticOperation * clone() const override;
};

class Leaf : public Term
{
	public:
		Leaf ( DataType type ) :
			Term ( false, std::move ( type ) )
		{ ; }

		virtual Leaf * clone() const override = 0;
};

class Constant : public Leaf
{
	public:
		explicit Constant ( DataType type ) :
			Leaf ( std::move ( type ) )
		{ ; }

		virtual Constant * clone() const override = 0;
};

class IntConstant : public Constant
{
	private:
		int _value;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		explicit IntConstant ( int value );
		virtual ~IntConstant() = default;

		virtual IntConstant * clone() const override;
};

// Constant of arbitrary type, stored in string
class UserConstant : public Constant
{
	private:
		std::string _value;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		explicit UserConstant ( DataType type, std::string  & value );
		explicit UserConstant ( DataType type, std::string && value );
		UserConstant ( const UserConstant & orig );
		UserConstant ( UserConstant && old );
		virtual ~UserConstant() = default;

		virtual UserConstant * clone() const override;
};

class VariableReference : public Leaf
{
	private:
		const Variable * _var;
		const bool       _primed;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		VariableReference ( const Variable &var, bool primed );
		virtual ~VariableReference() = default;

		bool primed() const { return _primed; }
		const Variable & variable () const { return *_var; }

		virtual VariableReference * clone() const override;
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
