#ifndef NTS_INLINER_HPP_
#define NTS_INLINER_HPP_
#pragma once

#include "variables.hpp"
#include "logic.hpp"
#include "nts.hpp"

/**
 * @brief Substitute variables inside formula with their shadow variables.
 * @pre Each variable in formula must have 'user_data' set to nullptr
 * or pointing to another variable.
 * @post Occurences of variables, which had 'user data' pointing to some other variable,
 * are substituted with the other variable. Nothing else is modified.
 */
void substitute_variables ( nts::Formula & f );
void substitute_variables ( nts::Term & t );

void annotate_with_origin ( nts::BasicNts & bn );

/**
 * @pre All variables in destination BasicNtses must have 'origin' annotation
 * @return number of inlined calls
 */
unsigned int inline_calls ( nts::BasicNts & bn, unsigned int first_var_id );

/**
 * @pre There is no recursion, neither direct nor indirect.
 */
void inline_calls_simple ( nts::Nts & nts );

struct visit_variable_uses
{
	const nts::VariableUse::visitor & _visitor;

	visit_variable_uses ( const nts::VariableUse::visitor & vis ) :
		_visitor ( vis )
	{ ; }

	void visit ( nts::Formula & f ) const;
	void visit ( nts::FormulaNot & f ) const;
	void visit ( nts::FormulaBop & fb ) const;

	void visit ( nts::QuantifiedFormula & qf ) const;
	void visit ( nts::AtomicProposition & ap ) const;
	void visit ( nts::Term & t ) const;

	void visit ( nts::Havoc & h ) const;
	void visit ( nts::ArrayWrite & aw ) const;
	void visit ( nts::Relation & r ) const;
	void visit ( nts::BooleanTerm & bt ) const;

	void visit ( nts::Leaf & lf ) const;
	void visit ( nts::MinusTerm & mt ) const;
	void visit ( nts::ArrayTerm & art ) const;
	void visit ( nts::ArithmeticOperation & aop ) const;

	void visit ( nts::CallTransitionRule & cr ) const;
	void visit ( nts::FormulaTransitionRule & fr ) const;
	void visit ( nts::TransitionRule & tr ) const;
};


#endif // NTS_INLINER_HPP_
