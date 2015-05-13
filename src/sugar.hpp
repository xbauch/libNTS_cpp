#ifndef SUGAR_HPP_
#define SUGAR_HPP_
#pragma once

#include <memory>

#include "logic.hpp"
#include "nts.hpp"

namespace nts {
namespace sugar
{

std::unique_ptr < nts::Relation > operator== (
		std::unique_ptr < nts::Term > &,
		std::unique_ptr < nts::Term > & );

std::unique_ptr < nts::Relation > operator> (
		std::unique_ptr < nts::Term > &,
		std::unique_ptr < nts::Term > & );

std::unique_ptr < nts::Relation > operator>= (
		std::unique_ptr < nts::Term > &,
		std::unique_ptr < nts::Term > & );

std::unique_ptr < nts::Relation > operator< (
		std::unique_ptr < nts::Term > &,
		std::unique_ptr < nts::Term > & );

std::unique_ptr < nts::Relation > operator<= (
		std::unique_ptr < nts::Term > &,
		std::unique_ptr < nts::Term > & );

std::unique_ptr < nts::Relation > operator!= (
		std::unique_ptr < nts::Term > &,
		std::unique_ptr < nts::Term > & );

// Compare to zero ( is negative )
std::unique_ptr < nts::Relation > operator< (
		std::unique_ptr < nts::Term > &,
		int );


nts::Relation & operator== ( nts::Term & t1, nts::Term & t2 );
nts::Relation & operator== ( nts::Term & t1, int t2 );

nts::Relation & operator> ( nts::Term & t1, nts::Term & t2 );
nts::Relation & operator> ( nts::Term & t1, int t2 );

nts::Relation & operator>= ( nts::Term & t1, nts::Term & t2 );
nts::Relation & operator>= ( nts::Term & t1, int t2 );

nts::Relation & operator< ( nts::Term & t1, nts::Term & t2 );
nts::Relation & operator< ( nts::Term & t1, int t2 );

nts::Relation & operator<= ( nts::Term & t1, nts::Term & t2 );
nts::Relation & operator<= ( nts::Term & t1, int t2 );

nts::ArithmeticOperation & operator+ ( nts::Term & t1, nts::Term & t2 );
nts::ArithmeticOperation & operator+ ( nts::Term & t1, int t2 );

nts::FormulaBop & operator== ( nts::Formula & f1, nts::Formula &f2 );

nts::FormulaBop & operator&& ( nts::Formula & f1, nts::Formula & f2 );

nts::FormulaNot & operator! ( nts::Formula & f );

// new instance of ThreadID
nts::ThreadID & tid();

nts::Havoc & havoc ();
nts::Havoc & havoc ( const std::vector < nts::Variable *> & vars );


// Reading VariableReference
nts::VariableReference & CURR ( nts::Variable & var );
nts::VariableReference & CURR ( nts::Variable * var );


// Writing VariableReference
nts::VariableReference & NEXT ( nts::Variable & var );
nts::VariableReference & NEXT ( nts::Variable * var );

nts::BooleanTerm & boolterm ( nts::VariableReference & );
nts::BooleanTerm & boolterm ( std::unique_ptr < nts::Leaf > && leaf );


class SugarTransitionStates
{
	private:
		nts::State & _from;
		nts::State & _to;

	public:
		SugarTransitionStates ( nts::State & from, nts::State & to );

		// Returns newly created transition, which becomes
		// an owner of given formula
		nts::Transition & operator () ( nts::Formula & f );
		nts::Transition & operator () ( std::unique_ptr < nts::Formula > f );

		nts::Transition & operator () ( nts::TransitionRule & ctr );
};

SugarTransitionStates operator ->* ( nts::State & from, nts::State & to );

// Array reading reference
class ArrRead
{
	private:
		nts::Variable & _arr_var;

	public:
		// Does not own anything
		ArrRead ( nts::Variable & arr_var );

		// Returned ArrayTerm becomes an owner of given term t
		nts::ArrayTerm & operator [] ( nts::Term & t );
};

// Use only as a temporary object
class ArrWriting
{
	private:
		nts::Variable & _arr_var;
		nts::Term * _idx;

	public:
		ArrWriting ( nts::Variable & arr_var, nts::Term & idx );

		// Use only once!
		nts::ArrayWrite & operator== ( nts::Term & value );
		nts::ArrayWrite & operator== ( int value );
};

class ArrWrite
{
	private:
		nts::Variable & _arr_var;

	public:
		ArrWrite ( nts::Variable & arr_var );

		ArrWriting operator [] ( nts::Term & idx );
};

}; // namespace sugar
}; // namespace nts

#endif // SUGAR_HPP_
