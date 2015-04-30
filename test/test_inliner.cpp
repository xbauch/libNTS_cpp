#include <iostream>

#include "nts.hpp"
#include "logic.hpp"
#include "sugar.hpp"

#include "inliner.hpp"

using namespace nts;
using namespace nts::sugar;

using std::cout;

void test_inlining()
{
	auto caller = new BasicNts ( "caller" );
	auto callee = new BasicNts ( "callee" );

	const DataType dt_int = DataType ( ScalarType::Integer() );
	auto callee_in  = new Variable ( dt_int, "var_in"  );
	auto callee_out = new Variable ( dt_int, "var_out" );
	callee_in ->insert_param_in_to ( *callee );
	callee_out->insert_param_out_to ( *callee );
	
	auto callee_initial = new State ( "si" );
	auto callee_final   = new State ( "sf" );
	auto callee_error   = new State ( "se" );

	callee_initial -> is_initial () = true;
	callee_final   -> is_final   () = true;
	callee_error   -> is_error   () = true;

	callee_initial -> insert_to ( *callee );
	callee_final   -> insert_to ( *callee );
	callee_error   -> insert_to ( *callee );

	// Transition to error state
	auto & f_e = havoc();
	auto & t_e = ( *callee_initial ->* *callee_error ) ( f_e );
	t_e.insert_to ( *callee );

	// Transition to final state
	auto & f_fin = ( NEXT ( callee_out ) == (CURR ( callee_in ) + 3 ) )
		&& havoc ( { callee_out } );
	auto & t_fin = ( *callee_initial ->* *callee_final ) ( f_fin );
	t_fin.insert_to ( *callee );


	auto result = new Variable ( dt_int, "result" );
	result->insert_to ( *caller );

	auto caller_initial = new State ( "c_si" );
	auto caller_final   = new State ( "c_sf" );
	caller_initial->insert_to ( *caller );
	caller_final  ->insert_to ( *caller );

	auto * ctr1 = new CallTransitionRule (
			*callee,
			{ new IntConstant ( 5 ) },
			{ result }
	);

	auto & t_call_1 = ( *caller_initial ->* *caller_final ) ( *ctr1 );
	t_call_1.insert_to ( *caller );

	auto * ctr2 = new CallTransitionRule (
			*callee,
			{ new IntConstant ( 19 ) },
			{ result }
	);

	auto & t_call_2 = ( *caller_initial ->* *caller_final ) ( *ctr2 );
	t_call_2.insert_to ( *caller );

	auto * nts = new Nts ( "toplevel" );
	callee->insert_to ( *nts );
	caller->insert_to ( *nts );

	auto * inst = new Instance ( caller, 7 );
	inst->insert_to ( *nts );

	cout << "** Original **\n";
	cout << *nts;

	annotate_with_origin ( *callee );
	annotate_with_origin ( *caller );
	inline_calls_simple ( *nts );

	cout << "** After **\n";
	cout << *nts;

	delete nts;
}


int main()
{
	test_inlining();
	return 0;
}
