#ifndef NTS_VARIABLES_HPP_
#define NTS_VARIABLES_HPP_
#pragma once

#include <vector>
#include <list>

namespace nts
{

class Variable;
class VariableUse;
class VariableReference;
class ArrayWrite;
class Havoc;
class CallTransitionRule;

// Used by variable to point to its uses.
using VariableUsesList = std::list < VariableUse * >;

// Everyone who uses a variable s
// Who uses this variable
// Each variable has a list of this
class VariableUse
{
	public:
		// Who uses the variable?
		enum class UserType
		{
			VariableReference,
			ArrayWrite,
			CallTransitionRule,
			Havoc
		};

		union UserPtr
		{
			void               * raw;
			VariableReference  * vref;
			ArrayWrite         * arr_wr;
			CallTransitionRule * ctr;
			Havoc              * hvc;
		};

	private:
		VariableUsesList::iterator _pos;
		Variable * _var;

	public:
		// Does this usage modify its value?
		const bool     modifying;
		// Points to one who uses the variable
		const UserType user_type;
		UserPtr        user_ptr;

		// By using these constructors caller specify
		// who uses the variable. This information
		// is stored in 'user_type' member field
		// and it is forever!
		explicit VariableUse ( VariableReference & vref, bool modify );
		explicit VariableUse ( ArrayWrite & arr_wr );
		explicit VariableUse ( CallTransitionRule & ctr );
		explicit VariableUse ( Havoc & havc );

		VariableUse ( UserType type, UserPtr ptr, bool modify );

		/**
		 * What would this mean? user uses the same variable twice?
		 */
		VariableUse ( const VariableUse & orig ) = delete;

		/**
		 * User remains the same, moves only the information
		 * about his using the variable.
		 */
		VariableUse ( VariableUse && old );

		~VariableUse();

		void set ( Variable * v );

		Variable * get() const { return _var; }

		/**
		 * @post: This Use does not point to any variable.
		 */
		Variable * release();

		Variable * operator->() const { return _var; }
		Variable & operator* () const { return *_var; }

		VariableUse & operator= ( Variable * v );
		VariableUse & operator= ( const VariableUse & orig );
		VariableUse & operator= ( VariableUse && old );

		explicit operator bool () const { return _var; }
		explicit operator const Variable * () const { return _var; }
		explicit operator Variable * () { return _var; }
};

/**
 * @brief Manages uses of variable.
 * If an user of variable needs to use multiple variables (e.g. Havoc ),
 * it can use this class.
 */
class VariableUseContainer : private std::vector < VariableUse >
{
	private:
		VariableUse::UserPtr  _ptr;
		VariableUse::UserType _type;

	public:
		const bool modifying;

		explicit VariableUseContainer ( VariableReference & vref, bool modify );
		explicit VariableUseContainer ( ArrayWrite & arr_wr );
		explicit VariableUseContainer ( CallTransitionRule & ctr );
		explicit VariableUseContainer ( Havoc & hvc );

		// This whould preserve owner, what we usually do not want
		VariableUseContainer ( const VariableUseContainer & orig ) = delete;
		VariableUseContainer ( VariableUseContainer && old ) = delete;

		VariableUseContainer & operator=
			( const VariableUseContainer & orig );

		VariableUseContainer & operator=
			( VariableUseContainer && old );

		using std::vector < VariableUse > :: begin;
		using std::vector < VariableUse > :: end;

		using std::vector < VariableUse > :: cbegin;
		using std::vector < VariableUse > :: cend;

		using std::vector < VariableUse > :: operator[];
		using std::vector < VariableUse > :: at;

		using std::vector < VariableUse > :: erase;
		using std::vector < VariableUse > :: clear;

		using std::vector < VariableUse > :: size;

		void push_back ( Variable * v );
};


/**
 * @brief Owns all inserted variables.
 * This class is used by Nts, BasicNts and QuantifiedVariableList
 * to hold all variables it owns.
 */
class VariableContainer : public std::list < Variable * >
{
	public:
		VariableContainer() = default;
		explicit VariableContainer ( std::list < Variable * > );
		VariableContainer ( const VariableContainer & ) = delete;
		VariableContainer ( VariableContainer && ) = delete;

		VariableContainer & operator= ( const VariableContainer & ) = delete;
		VariableContainer & operator= ( VariableContainer && old );

		~VariableContainer();
};



}; // namespace nts

#endif // NTS_VARIABLES_HPP
