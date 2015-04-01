#include <vector>
#include "nts.hpp"

using namespace std;
using namespace nts;


struct Example
{
	struct Nts2
	{
		BasicNts basic;
		BitVectorVariable arg_1;
		BitVectorVariable arg_2;
		BitVectorVariable arg_ret;


		Nts2() :
			arg_1   ( "arg_1", 4 ),
			arg_2   ( "arg_2", 8 ),
			arg_ret ( "arg_ret", 16 )
		{
			arg_1.insert_param_in_to ( &basic );
			arg_2.insert_param_in_to ( &basic );
			arg_ret.insert_param_out_to ( &basic );

		}
	};

	// This should call Nts2
	struct Nts1
	{
		BasicNts basic;
		BitVectorVariable var_1;
		BitVectorVariable var_2;
		BitVectorVariable var_3;

		CallTransition *ct1;

		Nts1 ( struct Nts2 *n ) :
			var_1   ( "var_1", 4 ),
			var_2   ( "var_2", 8 ),
			var_3   ( "var_3", 16 )
		{
			var_1.insert_to ( &basic );
			var_2.insert_to ( &basic );
			var_3.insert_to ( &basic );
			ct1 = new CallTransition ( &n->basic, { &var_1, &var_2}, { &var_3 } );

		}
		~Nts1()
		{
			delete ct1;
		}

	};

	BitVectorVariable bvvar[4];

	BasicNts nb[2];
	Nts toplevel_nts;

	vector<CallTransition *> ct;
	Transition tr[2];

	Example() :
		bvvar {
			BitVectorVariable ( "var1", 8 ),
			BitVectorVariable ( "var2", 16),
			BitVectorVariable ( "var3", 1 ),
			BitVectorVariable ( "var4", 4 )
		},

		tr {
			Transition ( Transition::Kind::Formula ),
			Transition ( Transition::Kind::Formula )
		}

	{
		// Two call transitions
		CallTransition *t = new CallTransition ( &nb[1], {}, {} );
		ct.push_back ( t );
		t = new CallTransition ( &nb[1], {}, {} );
		ct.push_back ( t );


		tr[0].insert_to ( &nb[0] );
		ct[0]->insert_to ( &nb[0] );
		ct[1]->insert_to ( &nb[0] );
		tr[1].insert_to ( &nb[0] );


		nb[0].insert_to ( &toplevel_nts );
		nb[1].insert_to ( &toplevel_nts );

		bvvar[0].insert_to ( & toplevel_nts );
		bvvar[1].insert_param_in_to ( & nb[1] );
		bvvar[2].insert_before ( bvvar[1] );

	}

	~Example()
	{
		for ( auto t : ct )
		{
			delete t;
		}
	}


	void try_callees()
	{
		auto i = nb[0].callees().begin();
		printf ( "%p == %p\n", ct[0], *i);
		i++;
		printf ( "%p == %p\n", ct[1], *i);
		i++;
		printf ( "end? %s\n", i == nb[0].callees().end() ? "yes" : "no" );
	}

	void try_callers()
	{
		auto i = nb[1].callers().begin();
		auto e = nb[1].callers().end();
		for (int j = 0; j < 5 && i != e; ++j, ++i )
		{
			printf( "%d: %p\n", j, (*i) );
		}
	}

	void examples()
	{
		Nts2 n2;
		Nts1 n1 ( &n2 );
	}

};

int main ( void )
{
	printf ( "Hello world\n" );

	Example e1;
	e1.try_callees();
	e1.try_callers();
	e1.examples();


	return 0;
}
