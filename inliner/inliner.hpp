#ifndef NTS_INLINER_HPP_
#define NTS_INLINER_HPP_
#pragma once

#include <libNTS/logic.hpp>
#include <libNTS/nts.hpp>

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

void inline_calls ( nts::BasicNts & bn );


#endif // NTS_INLINER_HPP_
