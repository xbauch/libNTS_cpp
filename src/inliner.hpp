#ifndef NTS_INLINER_HPP_
#define NTS_INLINER_HPP_
#pragma once

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


#endif // NTS_INLINER_HPP_
